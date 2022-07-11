/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcStats Class
 * @details   Collect and report information from /proc/stats
 *-
 */

#include "SysProcStat.h"
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

static constexpr int statCpuNamePos = 0;
static constexpr int statUserJiffiesPos = 1;
static constexpr int statSystemJiffiesPos = 3;

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcStat> mgr,
                          const SysProcStat::Request &request);
static bool doCollectAndSend(const std::shared_ptr<SysProcStat> mgr,
                             const SysProcStat::Request &request);

void CPUStat::updateStats(uint64_t newUserJiffies, uint64_t newSystemJiffies)
{
  if (m_lastUserJiffies == 0) {
    m_lastUserJiffies = newUserJiffies;
  }

  if (m_lastSystemJiffies == 0) {
    m_lastSystemJiffies = newSystemJiffies;
  }

  auto userJiffiesDiff =
      ((newUserJiffies - m_lastUserJiffies) > 0) ? (newUserJiffies - m_lastUserJiffies) : 0;
  auto sysJiffiesDiff =
      ((newSystemJiffies - m_lastSystemJiffies) > 0) ? (newSystemJiffies - m_lastSystemJiffies) : 0;

  m_lastUserJiffies = newUserJiffies;
  m_lastSystemJiffies = newSystemJiffies;

  auto timeNow = std::chrono::steady_clock::now();
  using USec = std::chrono::microseconds;

  if (m_lastUpdateTime.time_since_epoch().count() == 0) {
    m_userPercent = m_sysPercent = m_totalPercent = 0;
    m_lastUpdateTime = timeNow;
  } else {
    auto durationUs = std::chrono::duration_cast<USec>(timeNow - m_lastUpdateTime).count();
    m_lastUpdateTime = timeNow;

    m_userPercent = jiffiesToPercent(userJiffiesDiff, durationUs);
    m_sysPercent = jiffiesToPercent(sysJiffiesDiff, durationUs);
    m_totalPercent = jiffiesToPercent(userJiffiesDiff + sysJiffiesDiff, durationUs);
  }

  m_data.set_all(m_totalPercent);
  m_data.set_usr(m_userPercent);
  m_data.set_sys(m_sysPercent);
}

SysProcStat::SysProcStat(const std::shared_ptr<Options> options)
: m_options(options)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcStat", [this](const Request &request) { return requestHandler(request); });
}

auto SysProcStat::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcStat::enableEvents()
{
  App()->addEventSource(m_queue);
}

bool SysProcStat::update(void)
{
  if (getUpdatePending()) {
    return true;
  }

  SysProcStat::Request request = {.action = SysProcStat::Action::UpdateStats};
  bool status = pushRequest(request);

  if (status) {
    setUpdatePending(true);
  }

  return status;
}

auto SysProcStat::getCPUStat(const std::string &name) -> const std::shared_ptr<CPUStat>
{
  std::shared_ptr<CPUStat> cpuStat = nullptr;

  m_cpus.foreach ([this, &name, &cpuStat](const std::shared_ptr<CPUStat> &entry) {
    if (entry->getName() == name) {
      cpuStat = entry;
    }
  });

  return cpuStat;
}

auto SysProcStat::requestHandler(const Request &request) -> bool
{
  bool status = false;

  switch (request.action) {
  case SysProcStat::Action::UpdateStats:
    status = doUpdateStats(getShared(), request);
    setUpdatePending(false);
    break;
  case SysProcStat::Action::CollectAndSend:
    status = doCollectAndSend(getShared(), request);
    break;
  default:
    logError() << "Unknown action request";
    break;
  }

  return status;
}

static bool doUpdateStats(const std::shared_ptr<SysProcStat> mgr,
                          const SysProcStat::Request &request)
{
  std::ifstream statStream{"/proc/stat"};

  if (!statStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/stat file");
  }

  std::string line;
  while (std::getline(statStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    if (line.find("cpu") == std::string::npos) {
      break;
    }

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 4) {
      logError() << "Proc stat file parse error";
      return false;
    }

    auto updateCpuStatEntry = [&tokens](const std::shared_ptr<CPUStat> &entry) {
      uint64_t newUserJiffies = 0;
      uint64_t newSysJiffies = 0;

      try {
        newUserJiffies = std::stoul(tokens[statUserJiffiesPos].c_str());
        newSysJiffies = std::stoul(tokens[statSystemJiffiesPos].c_str());
      } catch (...) {
        logError() << "Cannot convert stat data to Jiffies";
        return;
      }

      entry->updateStats(newUserJiffies, newSysJiffies);
    };

    auto found = false;
    mgr->getCPUStatList().foreach (
        [&tokens, &found, updateCpuStatEntry](const std::shared_ptr<CPUStat> &entry) {
          if (entry->getName() == tokens[statCpuNamePos]) {
            updateCpuStatEntry(entry);
            found = true;
          }
        });

    if (!found) {
      std::shared_ptr<CPUStat> entry = std::make_shared<CPUStat>(tokens[statCpuNamePos]);
      logInfo() << "Adding new cpu core '" << entry->getName() << "' for statistics";
      mgr->getCPUStatList().append(entry);
      mgr->getCPUStatList().commit();
      updateCpuStatEntry(entry);
    }
  }

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcStat> mgr,
                             const SysProcStat::Request &request)
{
  tkm::msg::monitor::SysProcStat statEvent;
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_SysProcStat);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(currentTime.tv_sec);
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(currentTime.tv_sec);

  mgr->getCPUStatList().foreach ([&statEvent](const std::shared_ptr<CPUStat> &entry) {
    if (entry->getType() == CPUStat::StatType::Cpu) {
      statEvent.mutable_cpu()->CopyFrom(entry->getData());
    } else {
      statEvent.add_core()->CopyFrom(entry->getData());
    }
  });

  data.mutable_payload()->PackFrom(statEvent);
  request.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
