/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcWireless Class
 * @details   Collect and report information from /proc/net/wireless
 *-
 */

#include "SysProcWireless.h"
#include "Application.h"
#include "Logger.h"
#include "Monitor.pb.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcWireless> mgr,
                          const SysProcWireless::Request &request);
static bool doCollectAndSend(const std::shared_ptr<SysProcWireless> mgr,
                             const SysProcWireless::Request &request);

SysProcWireless::SysProcWireless(const std::shared_ptr<Options> options)
: m_options(options)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcWireless", [this](const Request &request) { return requestHandler(request); });
}

auto SysProcWireless::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcWireless::enableEvents()
{
  App()->addEventSource(m_queue);
}

bool SysProcWireless::update(void)
{
  if (getUpdatePending()) {
    return true;
  }

  SysProcWireless::Request request = {.action = SysProcWireless::Action::UpdateStats};
  bool status = pushRequest(request);

  if (status) {
    setUpdatePending(true);
  }

  return status;
}

auto SysProcWireless::requestHandler(const Request &request) -> bool
{
  bool status = false;

  switch (request.action) {
  case SysProcWireless::Action::UpdateStats:
    status = doUpdateStats(getShared(), request);
    setUpdatePending(false);
    break;
  case SysProcWireless::Action::CollectAndSend:
    status = doCollectAndSend(getShared(), request);
    break;
  default:
    logError() << "Unknown action request";
    break;
  }

  return status;
}

static bool doUpdateStats(const std::shared_ptr<SysProcWireless> mgr,
                          const SysProcWireless::Request &request)
{
  std::ifstream statStream{"/proc/net/wireless"};

  if (!statStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/net/wireless file");
  }

  std::string line;
  auto lines = 0;
  while (std::getline(statStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    // skip header lines
    if (lines++ < 2) {
      continue;
    }

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 11) {
      logError() << "Proc wireless file parse error";
      return false;
    }

    auto &name = tokens[0];
    if (!name.empty()) {
      name.pop_back();
    }

    auto updateWlanInterfaceEntry = [&tokens](const std::shared_ptr<WlanInterface> &entry) {
      entry->getData().set_status(tokens[1]);

      if (tokens[2].back() == '.') {
        tokens[2].pop_back();
      }
      entry->getData().set_quality_link(std::stoi(tokens[2]));

      if (tokens[3].back() == '.') {
        tokens[3].pop_back();
      }
      entry->getData().set_quality_level(std::stoi(tokens[3]));

      if (tokens[4].back() == '.') {
        tokens[4].pop_back();
      }
      entry->getData().set_quality_noise(std::stoi(tokens[4]));

      entry->getData().set_discarded_nwid(std::stoul(tokens[5]));
      entry->getData().set_discarded_crypt(std::stoul(tokens[6]));
      entry->getData().set_discarded_frag(std::stoul(tokens[7]));
      entry->getData().set_discarded_retry(std::stoul(tokens[8]));
      entry->getData().set_discarded_misc(std::stoul(tokens[9]));
      entry->getData().set_missed_beacon(std::stoul(tokens[10]));
    };

    auto found = false;
    mgr->getWlanInterfaceList().foreach (
        [&name, &found, updateWlanInterfaceEntry](const std::shared_ptr<WlanInterface> &entry) {
          if (entry->getData().name() == name) {
            updateWlanInterfaceEntry(entry);
            found = true;
          }
        });

    if (!found) {
      std::shared_ptr<WlanInterface> entry = std::make_shared<WlanInterface>(name);
      logInfo() << "Adding new wlan inteface with name=" << name;
      mgr->getWlanInterfaceList().append(entry);
      mgr->getWlanInterfaceList().commit();
      updateWlanInterfaceEntry(entry);
    }
  }

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcWireless> mgr,
                             const SysProcWireless::Request &request)
{
  tkm::msg::monitor::SysProcWireless sysProcWireless;
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_SysProcWireless);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(currentTime.tv_sec);
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(currentTime.tv_sec);

  mgr->getWlanInterfaceList().foreach (
      [&sysProcWireless](const std::shared_ptr<WlanInterface> &entry) {
        sysProcWireless.add_ifw()->CopyFrom(entry->getData());
      });

  data.mutable_payload()->PackFrom(sysProcWireless);
  request.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
