/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Helper methods
 * @details   Verious helper methods
 *-
 */

#include "Helpers.h"
#include "Logger.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#ifdef WITH_LXC
#include <lxc/lxccontainer.h>
#endif

#include "Envelope.pb.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace pbio = google::protobuf::io;

namespace tkm
{

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

static inline bool is_base64(unsigned char c)
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

#ifdef WITH_LXC
static auto getContainerNameForContext(const std::string &contPath, uint64_t ctxId) -> std::string;
#endif

auto base64Encode(unsigned char const *bytes_to_encode, unsigned int in_len) -> std::string
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while ((i++ < 3))
      ret += '=';
  }

  return ret;
}

auto base64Decode(std::string const &encoded_string) -> std::string
{
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_];
    in_++;
    if (i == 4) {
      for (i = 0; i < 4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++)
      char_array_4[j] = 0;

    for (j = 0; j < 4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++)
      ret += char_array_3[j];
  }

  return ret;
}

auto jnkHsh(const char *key) -> uint64_t
{
  uint64_t hash, i;

  for (hash = i = 0; i < strlen(key); ++i) {
    hash += (uint64_t) key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
}

bool sendCollectorDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor)
{
  tkm::msg::collector::Message message{};
  tkm::msg::Envelope envelope{};

  // We pack an empty descriptor to calculate envelope size
  message.set_type(tkm::msg::collector::Message_Type_Descriptor);
  message.mutable_data()->PackFrom(descriptor);
  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  unsigned char buffer[128]{};
  pbio::ArrayOutputStream outputArray(buffer, sizeof(buffer));
  pbio::CodedOutputStream codedOutput(&outputArray);

  auto envelopeSize = envelope.ByteSizeLong();
  codedOutput.WriteVarint32(envelopeSize);

  if (!envelope.SerializeToCodedStream(&codedOutput)) {
    return false;
  }

  if (send(fd, buffer, envelopeSize + sizeof(uint64_t), MSG_WAITALL) !=
      (static_cast<ssize_t>(envelopeSize + sizeof(uint64_t)))) {
    return false;
  }

  return true;
}

bool readCollectorDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor)
{
  tkm::msg::collector::Message message{};
  tkm::msg::Envelope envelope{};

  // We pack an empty descriptor to calculate envelope size
  message.set_type(tkm::msg::collector::Message_Type_Descriptor);
  message.mutable_data()->PackFrom(descriptor);
  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  unsigned char buffer[128]{};
  pbio::ArrayInputStream inputArray(buffer, sizeof(buffer));
  pbio::CodedInputStream codedInput(&inputArray);

  if (recv(fd, buffer, sizeof(uint64_t), MSG_WAITALL) != static_cast<ssize_t>(sizeof(uint64_t))) {
    return false;
  }

  uint32_t messageSize;
  codedInput.ReadVarint32(&messageSize);
  if (recv(fd, buffer + sizeof(uint64_t), messageSize, MSG_WAITALL) != messageSize) {
    return false;
  }

  codedInput.PushLimit(messageSize);
  if (!envelope.ParseFromCodedStream(&codedInput)) {
    return false;
  }

  envelope.mesg().UnpackTo(&message);
  if (message.type() != tkm::msg::collector::Message_Type_Descriptor) {
    return false;
  }

  message.data().UnpackTo(&descriptor);
  return true;
}

auto readLink(std::string const &path) -> std::string
{
  char buff[PATH_MAX] = {0};
  ssize_t len = ::readlink(path.c_str(), buff, sizeof(buff) - 1);

  if (len > 0) {
    return std::string(buff);
  }

  return std::string("NA");
}

auto getContextId(pid_t pid) -> uint64_t
{
  std::stringstream ctxStr;

  for (int i = 0; i < 10; i++) {
    std::string procPath{};

    switch (i) {
    case 0: /* cgroup */
      procPath = "/proc/" + std::to_string(pid) + "/ns/cgroup";
      break;
    case 1: /* ipc */
      procPath = "/proc/" + std::to_string(pid) + "/ns/ipc";
      break;
    case 2: /* mnt */
      procPath = "/proc/" + std::to_string(pid) + "/ns/mnt";
      break;
    case 3: /* net */
      procPath = "/proc/" + std::to_string(pid) + "/ns/net";
      break;
    case 4: /* pid */
      procPath = "/proc/" + std::to_string(pid) + "/ns/pid";
      break;
    case 5: /* pid_for_children */
      procPath = "/proc/" + std::to_string(pid) + "/ns/pid_for_children";
      break;
    case 6: /* time */
      procPath = "/proc/" + std::to_string(pid) + "/ns/time";
      break;
    case 7: /* time_for_children */
      procPath = "/proc/" + std::to_string(pid) + "/ns/time_for_children";
      break;
    case 8: /* user */
      procPath = "/proc/" + std::to_string(pid) + "/ns/user";
      break;
    case 9: /* uts */
      procPath = "/proc/" + std::to_string(pid) + "/ns/uts";
      break;
    default: /* never reached */
      break;
    }

    if (std::filesystem::exists(procPath)) {
      ctxStr << readLink(procPath);
    }
  }

  if (ctxStr.str().length() == 0) {
    logDebug() << "Invalid ctxStr";
  }

  return jnkHsh(ctxStr.str().c_str());
}

#ifdef WITH_LXC
static auto getContainerNameForContext(const std::string &contPath, uint64_t ctxId) -> std::string
{
  struct lxc_container **activeContainers = NULL;
  std::string contName{"unknown"};
  char **names = NULL;
  bool found = false;

  auto count = list_active_containers(contPath.c_str(), &names, &activeContainers);
  for (int i = 0; i < count && !found; i++) {
    struct lxc_container *container = activeContainers[i];
    const char *name = names[i];

    if (name == NULL || container == NULL) {
      continue;
    }

    if (container->is_running(container)) {
      auto pid = container->init_pid(container);
      if (getContextId(pid) == ctxId) {
        contName = std::string(name);
      }
    }
  }

  for (int i = 0; i < count; i++) {
    free(names[i]);
    lxc_container_put(activeContainers[i]);
  }

  return contName;
}
#endif

auto getContextName(const std::string &contPath, uint64_t ctxId) -> std::string
{
  if (ctxId == getContextId(1)) {
    return std::string{"root"};
  }
#ifdef WITH_LXC
  return getContainerNameForContext(contPath, ctxId);
#else
  return std::string("unknown");
#endif
}

} // namespace tkm
