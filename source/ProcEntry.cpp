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
#include "ProcAcct.h"
#include <cstdint>

namespace tkm::monitor
{

ProcEntry::ProcEntry(int pid, const std::string &name)
: m_name(name)
, m_pid(pid)
{
  m_timer = std::make_shared<Timer>("ProcEntry", [pid]() {
    if (!App()->getProcAcct()->requestTaskAcct(pid)) {
      App()->getRegistry()->remEntry(pid);
      return false;
    }
    return true;
  });
}

void ProcEntry::startMonitoring(unsigned int interval)
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

} // namespace tkm::monitor
