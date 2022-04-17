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

#include "ProcEntry.h"
#include "Application.h"

namespace tkm::monitor
{

ProcEntry::ProcEntry(int pid)
: m_pid(pid)
{
  if (TaskMonitor()->getOptions()->getFor(Options::Key::SkipIfNoClients) == "true") {
    m_skipIfNoClients = true;
  }

  m_timer = std::make_shared<Timer>("ProcEntry", [this]() {
    if (!TaskMonitor()->getNetServer()->hasClients() && m_skipIfNoClients) {
      return true;
    }
    return (TaskMonitor()->getManager()->getProcAcct()->requestTaskAcct(m_pid) != -1) ? true
                                                                                      : false;
  });
};

void ProcEntry::startMonitoring(int interval)
{
  m_pollInterval = interval;
  m_timer->start(interval, true);
  TaskMonitor()->addEventSource(m_timer);
}

void ProcEntry::disable(void)
{
  m_timer->stop();
  TaskMonitor()->remEventSource(m_timer);
}

auto ProcEntry::getUserCPUPercent(uint64_t cpuTime) -> int
{
  if (m_lastUserCPUTime == 0) {
    m_lastUserCPUTime = cpuTime;
  }

  auto userCPUPercent = ((cpuTime - m_lastUserCPUTime) * 100) / m_pollInterval;
  m_lastUserCPUTime = cpuTime;

  return userCPUPercent;
}

auto ProcEntry::getSystemCPUPercent(uint64_t cpuTime) -> int
{
  if (m_lastSystemCPUTime == 0) {
    m_lastSystemCPUTime = cpuTime;
  }

  auto sysCPUPercent = ((cpuTime - m_lastSystemCPUTime) * 100) / m_pollInterval;
  m_lastSystemCPUTime = cpuTime;

  return sysCPUPercent;
}

} // namespace tkm::monitor
