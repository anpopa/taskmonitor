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

#include <TaskMonitor.h>

#include "ICollector.h"
#include "IDataSource.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{

struct CPUStat : public std::enable_shared_from_this<CPUStat> {
public:
  enum class StatType { Cpu, Core };
  explicit CPUStat(const std::string &name)
  {
    m_sysHZ = sysconf(_SC_CLK_TCK);
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

  void updateStats(uint64_t newUserJiffies, uint64_t newSystemJiffies);
  auto getData(void) -> tkm::msg::monitor::CPUStat & { return m_data; }
  auto getLastUserCPUTime(void) -> uint64_t { return (m_lastUserJiffies * 1000000 / m_sysHZ); }
  auto getLastSystemCPUTime(void) -> uint64_t { return (m_lastSystemJiffies * 1000000 / m_sysHZ); }

private:
  auto jiffiesToPercent(uint64_t jiffies, uint64_t durationUsec) -> uint16_t
  {
    return ((jiffies * 1000000 / m_sysHZ) * 100) / durationUsec;
  }

private:
  tkm::msg::monitor::CPUStat m_data;
  std::chrono::time_point<std::chrono::steady_clock> m_lastUpdateTime{};
  uint64_t m_lastUserJiffies = 0;
  uint64_t m_lastSystemJiffies = 0;
  int m_totalPercent = 0;
  int m_userPercent = 0;
  int m_sysPercent = 0;
  int m_sysHZ = 0;
  StatType m_type = StatType::Cpu;
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
  void enableEvents();
  bool update(void) final;

private:
  bool requestHandler(const Request &request);

private:
  bswi::util::SafeList<std::shared_ptr<CPUStat>> m_cpus{"StatCPUList"};
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
};

} // namespace tkm::monitor
