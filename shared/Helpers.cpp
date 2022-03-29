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

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>

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

bool sendClientDescriptor(int fd, tkm::msg::client::Descriptor &descriptor)
{
    tkm::msg::client::Message message {};
    tkm::msg::Envelope envelope {};

    // We pack an empty descriptor to calculate envelope size
    message.set_type(tkm::msg::client::Message_Type_Descriptor);
    message.mutable_data()->PackFrom(descriptor);
    envelope.mutable_mesg()->PackFrom(message);
    envelope.set_target(tkm::msg::Envelope_Recipient_Server);
    envelope.set_origin(tkm::msg::Envelope_Recipient_Client);

    unsigned char buffer[128] {};
    pbio::ArrayOutputStream outputArray(buffer, sizeof(buffer));
    pbio::CodedOutputStream codedOutput(&outputArray);

    auto envelopeSize = envelope.ByteSizeLong();
    codedOutput.WriteVarint32(envelopeSize);

    if (!envelope.SerializeToCodedStream(&codedOutput)) {
        return false;
    }

    if (send(fd, buffer, envelopeSize + sizeof(uint64_t), MSG_WAITALL)
        != (static_cast<ssize_t>(envelopeSize + sizeof(uint64_t)))) {
        return false;
    }

    return true;
}

bool readClientDescriptor(int fd, tkm::msg::client::Descriptor &descriptor)
{
    tkm::msg::client::Message message {};
    tkm::msg::Envelope envelope {};

    // We pack an empty descriptor to calculate envelope size
    message.set_type(tkm::msg::client::Message_Type_Descriptor);
    message.mutable_data()->PackFrom(descriptor);
    envelope.mutable_mesg()->PackFrom(message);
    envelope.set_target(tkm::msg::Envelope_Recipient_Server);
    envelope.set_origin(tkm::msg::Envelope_Recipient_Client);

    unsigned char buffer[128] {};
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
    if (message.type() != tkm::msg::client::Message_Type_Descriptor) {
        return false;
    }

    message.data().UnpackTo(&descriptor);
    return true;
}

} // namespace tkm
