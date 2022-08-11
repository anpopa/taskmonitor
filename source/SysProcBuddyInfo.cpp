/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcBuddyInfo Class
 * @details   Collect and report information from /proc/buddyinfo
 *-
 */

#include "SysProcBuddyInfo.h"
#include "Application.h"

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcBuddyInfo> mgr,
                          const SysProcBuddyInfo::Request &request);
static bool doCollectAndSend(const std::shared_ptr<SysProcBuddyInfo> mgr,
                             const SysProcBuddyInfo::Request &request);

SysProcBuddyInfo::SysProcBuddyInfo(const std::shared_ptr<Options> options)
: m_options(options)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcBuddyInfo", [this](const Request &request) { return requestHandler(request); });
}

auto SysProcBuddyInfo::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcBuddyInfo::enableEvents()
{
  App()->addEventSource(m_queue);
}

bool SysProcBuddyInfo::update(void)
{
  if (getUpdatePending()) {
    return true;
  }

  SysProcBuddyInfo::Request request = {.action = SysProcBuddyInfo::Action::UpdateStats};
  bool status = pushRequest(request);

  if (status) {
    setUpdatePending(true);
  }

  return status;
}

auto SysProcBuddyInfo::requestHandler(const Request &request) -> bool
{
  bool status = false;

  switch (request.action) {
  case SysProcBuddyInfo::Action::UpdateStats:
    status = doUpdateStats(getShared(), request);
    setUpdatePending(false);
    break;
  case SysProcBuddyInfo::Action::CollectAndSend:
    status = doCollectAndSend(getShared(), request);
    break;
  default:
    logError() << "Unknown action request";
    break;
  }

  return status;
}

static bool doUpdateStats(const std::shared_ptr<SysProcBuddyInfo> mgr,
                          const SysProcBuddyInfo::Request &request)
{
  std::ifstream statStream{"/proc/buddyinfo"};

  if (!statStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/buddyinfo file");
  }

  std::string line;
  while (std::getline(statStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    if (line.find("zone") == std::string::npos) {
      continue;
    }

    auto cnt = 0;
    auto zoneMarker = 0;
    while (ss >> buf) {
      if ((zoneMarker == 0) && (buf == "zone")) {
        zoneMarker = cnt;
      }
      cnt++;
      tokens.push_back(buf);
    }

    if (cnt < zoneMarker + 3) {
      logError() << "Proc buddyinfo file parse error";
      return false;
    }

    auto updateBuddyInfoEntry = [zoneMarker, &tokens](const std::shared_ptr<BuddyInfo> &entry) {
      std::string data{};
      for (int i = zoneMarker + 2; i < tokens.size(); i++) {
        data += tokens[i] + " ";
      }
      entry->getData().set_data(data);
    };

    std::string nameToken{};
    for (int i = 0; i < zoneMarker; i++) {
      nameToken += tokens[i];
    }
    // remove comma after name
    if (!nameToken.empty()) {
      nameToken.pop_back();
    }

    const auto &zoneToken = tokens[zoneMarker + 1];

    auto found = false;
    mgr->getBuddyInfoList().foreach ([&nameToken, &zoneToken, &found, updateBuddyInfoEntry](
                                         const std::shared_ptr<BuddyInfo> &entry) {
      if (entry->getHash() == tkm::jnkHsh(std::string(nameToken + zoneToken).c_str())) {
        updateBuddyInfoEntry(entry);
        found = true;
      }
    });

    if (!found) {
      std::shared_ptr<BuddyInfo> entry = std::make_shared<BuddyInfo>(nameToken, zoneToken);
      logInfo() << "Adding new buddyinfo entry with name=" << nameToken << " zone=" << zoneToken;
      mgr->getBuddyInfoList().append(entry);
      mgr->getBuddyInfoList().commit();
      updateBuddyInfoEntry(entry);
    }
  }

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcBuddyInfo> mgr,
                             const SysProcBuddyInfo::Request &request)
{
  tkm::msg::monitor::SysProcBuddyInfo info;
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_SysProcBuddyInfo);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(currentTime.tv_sec);
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(currentTime.tv_sec);

  mgr->getBuddyInfoList().foreach ([&info](const std::shared_ptr<BuddyInfo> &entry) {
    info.add_node()->CopyFrom(entry->getData());
  });

  data.mutable_payload()->PackFrom(info);
  request.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
