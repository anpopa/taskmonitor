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

#pragma once

#include <cstdint>
#include <taskmonitor/taskmonitor.h>

#include "ICollector.h"
#include "IDataSource.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{

struct CPUStatData {
  enum class DataField {
    UserTime,
    NiceTime,
    SystemTime,
    IdleTime,
    IOWaitTime,
    IRQTime,
    SoftIRQTime,
    StealTime,
    GuestTime,
    GuestNiceTime
  };

  uint64_t userTime = 0;
  uint64_t niceTime = 0;
  uint64_t systemTime = 0;
  uint64_t idleTime = 0;
  uint64_t ioWaitTime = 0;
  uint64_t irqTime = 0;
  uint64_t softIRQTime = 0;
  uint64_t stealTime = 0;
  uint64_t guestTime = 0;
  uint64_t guestNiceTime = 0;

  uint64_t getTotalTime()
  {
    return userTime + niceTime + systemTime + idleTime + ioWaitTime + irqTime + softIRQTime +
           stealTime + guestTime + guestNiceTime;
  }

  CPUStatData getDiff(const CPUStatData &prev) const
  {
    return {.userTime = userTime - prev.userTime,
            .niceTime = niceTime - prev.niceTime,
            .systemTime = systemTime - prev.systemTime,
            .idleTime = idleTime - prev.idleTime,
            .ioWaitTime = ioWaitTime - prev.ioWaitTime,
            .irqTime = irqTime - prev.irqTime,
            .softIRQTime = softIRQTime - prev.softIRQTime,
            .stealTime = stealTime - prev.stealTime,
            .guestTime = guestTime - prev.guestTime,
            .guestNiceTime = guestNiceTime - prev.guestNiceTime};
  }

  void copyFrom(const CPUStatData &src)
  {
    userTime = src.userTime;
    niceTime = src.niceTime;
    systemTime = src.systemTime;
    idleTime = src.idleTime;
    ioWaitTime = src.ioWaitTime;
    irqTime = src.irqTime;
    softIRQTime = src.softIRQTime;
    stealTime = src.stealTime;
    guestTime = src.guestTime;
    guestNiceTime = src.guestNiceTime;
  }

  void clear(void)
  {
    userTime = 0;
    niceTime = 0;
    systemTime = 0;
    idleTime = 0;
    ioWaitTime = 0;
    irqTime = 0;
    softIRQTime = 0;
    stealTime = 0;
    guestTime = 0;
    guestNiceTime = 0;
  }

  uint64_t getPercent(DataField type)
  {
    switch (type) {
    case DataField::UserTime:
      return userTime * 100 / getTotalTime();
    case DataField::NiceTime:
      return niceTime * 100 / getTotalTime();
    case DataField::SystemTime:
      return systemTime * 100 / getTotalTime();
    case DataField::IdleTime:
      return idleTime * 100 / getTotalTime();
    case DataField::IOWaitTime:
      return ioWaitTime * 100 / getTotalTime();
    case DataField::IRQTime:
      return irqTime * 100 / getTotalTime();
    case DataField::SoftIRQTime:
      return softIRQTime * 100 / getTotalTime();
    case DataField::StealTime:
      return stealTime * 100 / getTotalTime();
    case DataField::GuestTime:
      return guestTime * 100 / getTotalTime();
    case DataField::GuestNiceTime:
      return guestNiceTime * 100 / getTotalTime();
    default:
      break;
    }
    return 0;
  }
};

struct CPUStat : public std::enable_shared_from_this<CPUStat> {
public:
  enum class StatType { Cpu, Core };
  explicit CPUStat(const std::string &name)
  {
    m_data.set_name(name);
    if (name == "cpu") {
      m_type = StatType::Cpu;
    } else {
      m_type = StatType::Core;
    }
  };
  ~CPUStat() = default;

public:
  CPUStat(CPUStat const &) = delete;
  void operator=(CPUStat const &) = delete;

  auto getName(void) -> const std::string & { return m_data.name(); }
  auto getType(void) -> StatType { return m_type; }
  void updateStats(const CPUStatData &data);
  auto getData(void) -> tkm::msg::monitor::CPUStat & { return m_data; }

private:
  std::chrono::time_point<std::chrono::steady_clock> m_lastUpdateTime{};
  tkm::msg::monitor::CPUStat m_data;
  StatType m_type = StatType::Cpu;
  CPUStatData m_last;
};

class SysProcStat : public IDataSource, public std::enable_shared_from_this<SysProcStat>
{
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit SysProcStat(const std::shared_ptr<Options> options);
  virtual ~SysProcStat() = default;

public:
  SysProcStat(SysProcStat const &) = delete;
  void operator=(SysProcStat const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcStat> { return shared_from_this(); }
  auto getCPUStat(const std::string &name) -> const std::shared_ptr<CPUStat>;
  auto getCPUStatList() -> bswi::util::SafeList<std::shared_ptr<CPUStat>> & { return m_cpus; }
  auto pushRequest(SysProcStat::Request &request) -> int;
  void setEventSource(bool enabled = true);
  bool update(void) final;

private:
  bool requestHandler(const Request &request);

private:
  bswi::util::SafeList<std::shared_ptr<CPUStat>> m_cpus{"StatCPUList"};
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
};

} // namespace tkm::monitor
