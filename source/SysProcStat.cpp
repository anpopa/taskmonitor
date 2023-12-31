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
#include <cstring>
#include <string>

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcStat> mgr);
static bool doCollectAndSend(const std::shared_ptr<SysProcStat> mgr,
                             const SysProcStat::Request &request);

void CPUStat::updateStats(const CPUStatData &data)
{
  if (m_last.getTotalTime() == 0) {
    m_last.copyFrom(data);
  } else {
    auto diffData = data.getDiff(m_last);
    auto usr = static_cast<uint32_t>(diffData.getPercent(CPUStatData::DataField::UserTime));
    auto sys = static_cast<uint32_t>(diffData.getPercent(CPUStatData::DataField::SystemTime));
    auto iow = static_cast<uint32_t>(diffData.getPercent(CPUStatData::DataField::IOWaitTime));
    // During resume the computed time can be invalid depending when we sleep, if not valid
    // we clear m_last and copy only on the first valid data in the next updates
    if ((usr + sys + iow) <= 100) {
      m_last.copyFrom(data);
      m_data.set_usr(usr);
      m_data.set_sys(sys);
      m_data.set_iow(iow);
      m_data.set_all(usr + sys + iow);
    } else {
      m_last.clear();
    }
  }
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

void SysProcStat::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(m_queue);
  } else {
    App()->remEventSource(m_queue);
  }
}

bool SysProcStat::update(void)
{
  if (getUpdatePending()) {
    return true;
  }

  SysProcStat::Request request = {.action = SysProcStat::Action::UpdateStats, .collector = nullptr};
  bool status = pushRequest(request);

  if (status) {
    setUpdatePending(true);
  }

  return status;
}

auto SysProcStat::getCPUStat(const std::string &name) -> const std::shared_ptr<CPUStat>
{
  std::shared_ptr<CPUStat> cpuStat = nullptr;

  m_cpus.foreach ([&name, &cpuStat](const std::shared_ptr<CPUStat> &entry) {
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
    status = doUpdateStats(getShared());
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

static bool doUpdateStats(const std::shared_ptr<SysProcStat> mgr)
{
  std::ifstream statStream{"/proc/stat"};

  if (!statStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/stat file");
  }

  std::string line;
  while (std::getline(statStream, line)) {
    // cpu lines are at the start
    if (line.find("cpu") == std::string::npos) {
      break;
    }

    char name[64] = {0};
    CPUStatData data{};
    auto cnt = ::sscanf(line.c_str(),
                        "%s   %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                        name,
                        &data.userTime,
                        &data.niceTime,
                        &data.systemTime,
                        &data.idleTime,
                        &data.ioWaitTime,
                        &data.irqTime,
                        &data.softIRQTime,
                        &data.stealTime,
                        &data.guestTime,
                        &data.guestNiceTime);

    if (cnt < 11) {
      logError() << "Proc stat file parse error";
      return false;
    }

    auto found = false;
    mgr->getCPUStatList().foreach ([&data, &name, &found](const std::shared_ptr<CPUStat> &entry) {
      if (entry->getName() == name) {
        entry->updateStats(data);
        found = true;
      }
    });

    if (!found) {
      std::shared_ptr<CPUStat> entry = std::make_shared<CPUStat>(name);
      entry->updateStats(data);

      logDebug() << "Adding new cpu core '" << entry->getName() << "' for statistics";
      mgr->getCPUStatList().append(entry);
      mgr->getCPUStatList().commit();
    }
  }

#ifdef WITH_STARTUP_DATA
  if (App()->getStartupData() != nullptr) {
    if (!App()->getStartupData()->expired()) {
      tkm::msg::monitor::SysProcStat statData;

      mgr->getCPUStatList().foreach ([&statData](const std::shared_ptr<CPUStat> &entry) {
        if (entry->getType() == CPUStat::StatType::Cpu) {
          statData.mutable_cpu()->CopyFrom(entry->getData());
        } else {
          statData.add_core()->CopyFrom(entry->getData());
        }
      });

      App()->getStartupData()->addCpuData(statData);
    }
  }
#endif

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
  data.set_system_time_sec(static_cast<uint64_t>(currentTime.tv_sec));
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(static_cast<uint64_t>(currentTime.tv_sec));

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
