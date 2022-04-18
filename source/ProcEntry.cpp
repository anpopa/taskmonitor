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
#include <cstdint>

namespace tkm::monitor
{

ProcEntry::ProcEntry(int pid)
: m_pid(pid)
{
  if (App()->getOptions()->getFor(Options::Key::SkipIfNoClients) == "true") {
    m_skipIfNoClients = true;
  }

  m_timer = std::make_shared<Timer>("ProcEntry", [this]() {
    if (!App()->getTCPServer()->hasClients() && m_skipIfNoClients) {
      return true;
    }
    return (App()->getDispatcher()->getProcAcct()->requestTaskAcct(m_pid) != -1) ? true : false;
  });
};

void ProcEntry::startMonitoring(int interval)
{
  m_pollInterval = interval;
  m_timer->start(interval, true);
  App()->addEventSource(m_timer);
}

void ProcEntry::disable(void)
{
  m_timer->stop();
  App()->remEventSource(m_timer);
}

auto ProcEntry::getCPUPercent(uint64_t utime, uint64_t stime) -> uint32_t
{
  if (m_lastCPUTime == 0) {
    m_lastCPUTime = utime + stime;
  }

  uint32_t cpuPercent = (((utime + stime) - m_lastCPUTime) * 100) / m_pollInterval;
  m_lastCPUTime = utime + stime;

  return cpuPercent;
}

} // namespace tkm::monitor
