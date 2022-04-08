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

#include <time.h>
#include <unistd.h>

#include "Options.h"
#include "Server.pb.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/SafeList.h"
#include "../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{

struct CPUStat : public std::enable_shared_from_this<CPUStat> {
public:
  explicit CPUStat(const std::string &name, size_t usecInterval)
  : m_usecInterval(usecInterval)
  {
    m_sysHZ = sysconf(_SC_CLK_TCK);
    m_data.set_name(name);
  };
  ~CPUStat() = default;

public:
  CPUStat(CPUStat const &) = delete;
  void operator=(CPUStat const &) = delete;

  auto getName(void) -> const std::string & { return m_data.name(); }

  void updateStats(uint64_t newUserJiffies, uint64_t newSystemJiffies);
  auto getData(void) -> tkm::msg::server::CPUStat & { return m_data; }
  void printStats(void);

  auto getPollInterval(void) -> int { return m_usecInterval; }
  auto getLastUserCPUTime(void) -> uint64_t { return (m_lastUserJiffies * 1000000 / m_sysHZ); }
  auto getLastSystemCPUTime(void) -> uint64_t { return (m_lastSystemJiffies * 1000000 / m_sysHZ); }

private:
  auto jiffiesToPercent(uint64_t jiffies) -> int
  {
    return ((jiffies * 1000000 / m_sysHZ) * 100) / m_usecInterval;
  }

private:
  uint64_t m_lastUserJiffies = 0;
  uint64_t m_lastSystemJiffies = 0;
  int m_totalPercent = 0;
  int m_userPercent = 0;
  int m_sysPercent = 0;
  int m_sysHZ = 0;
  size_t m_usecInterval = 0;
  tkm::msg::server::CPUStat m_data;
};

class SysProcStat : public std::enable_shared_from_this<SysProcStat>
{
public:
  explicit SysProcStat(std::shared_ptr<Options> &options);
  ~SysProcStat() = default;

public:
  SysProcStat(SysProcStat const &) = delete;
  void operator=(SysProcStat const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcStat> { return shared_from_this(); }
  void enableEvents();

  void startMonitoring(void);
  void disable(void);
  auto getCPUStat(const std::string &name) -> const std::shared_ptr<CPUStat>;

private:
  bool processOnTick(void);

private:
  bswi::util::SafeList<std::shared_ptr<CPUStat>> m_cpus{"StatCPUList"};
  std::unique_ptr<std::ifstream> m_file = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  std::shared_ptr<Timer> m_timer = nullptr;
  bool m_printToLog = true;
  size_t m_usecInterval = 0;
};

} // namespace tkm::monitor
