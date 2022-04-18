/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcEntry Class
 * @details   Represent per process statistics
 *-
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ProcEntry
{
public:
  explicit ProcEntry(int pid);
  ~ProcEntry() = default;

public:
  ProcEntry(ProcEntry const &) = delete;
  void operator=(ProcEntry const &) = delete;

public:
  auto getPid() -> int { return m_pid; }
  void startMonitoring(int interval);
  void disable(void);
  auto getCPUPercent(uint64_t utime, uint64_t stime) -> uint32_t;

private:
  std::shared_ptr<Timer> m_timer = nullptr;
  uint64_t m_lastCPUTime = 0;
  bool m_skipIfNoClients = false;
  int m_pollInterval = 0;
  int m_pid = 0;
};

} // namespace tkm::monitor
