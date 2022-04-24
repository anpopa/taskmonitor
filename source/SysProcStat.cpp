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
#include "Monitor.pb.h"

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

static bool doUpdateStats(const std::shared_ptr<SysProcStat> &mgr,
                          const SysProcStat::Request &request);
static bool doCollectAndSend(const std::shared_ptr<SysProcStat> &mgr,
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

  m_userPercent = jiffiesToPercent(userJiffiesDiff);
  m_sysPercent = jiffiesToPercent(sysJiffiesDiff);
  m_totalPercent = jiffiesToPercent(userJiffiesDiff + sysJiffiesDiff);

  m_data.set_all(m_totalPercent);
  m_data.set_usr(m_userPercent);
  m_data.set_sys(m_sysPercent);
}

SysProcStat::SysProcStat(std::shared_ptr<Options> &options)
: m_options(options)
{
  try {
    m_usecInterval = std::stol(m_options->getFor(Options::Key::StatPollInterval));
  } catch (...) {
    throw std::runtime_error("Fail process StatPollInterval");
  }

  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcStat", [this](const Request &request) { return requestHandler(request); });
  m_timer = std::make_shared<Timer>("SysProcStatTimer", [this]() {
    SysProcStat::Request request = {.action = SysProcStat::Action::UpdateStats};
    return requestHandler(request);
  });
}

auto SysProcStat::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcStat::enableEvents()
{
  // Module queue
  App()->addEventSource(m_queue);

  // Update timer
  m_timer->start(m_usecInterval, true);
  App()->addEventSource(m_timer);
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
  switch (request.action) {
  case SysProcStat::Action::UpdateStats:
    return doUpdateStats(getShared(), request);
  case SysProcStat::Action::CollectAndSend:
    return doCollectAndSend(getShared(), request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doUpdateStats(const std::shared_ptr<SysProcStat> &mgr,
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

    auto updateCpuStatEntry = [tokens](const std::shared_ptr<CPUStat> &entry) {
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
        [tokens, &found, updateCpuStatEntry](const std::shared_ptr<CPUStat> &entry) {
          if (entry->getName() == tokens[statCpuNamePos].c_str()) {
            updateCpuStatEntry(entry);
            found = true;
          }
        });

    if (!found) {
      logInfo() << "Adding new cpu core for statistics";
      std::shared_ptr<CPUStat> entry =
          std::make_shared<CPUStat>(tokens[statCpuNamePos].c_str(), mgr->getUsecInterval());

      mgr->getCPUStatList().append(entry);
      mgr->getCPUStatList().commit();
      updateCpuStatEntry(entry);
    }
  }

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcStat> &mgr,
                             const SysProcStat::Request &request)
{
  mgr->getCPUStatList().foreach ([&request](const std::shared_ptr<CPUStat> &entry) {
    tkm::msg::monitor::Data data;

    data.set_what(tkm::msg::monitor::Data_What_SysProcStat);
    data.set_timestamp(time(NULL));

    data.mutable_payload()->PackFrom(entry->getData());
    request.collector->sendData(data);
  });

  return true;
}

} // namespace tkm::monitor
