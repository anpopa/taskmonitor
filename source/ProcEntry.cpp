/*
 * SPDX license identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2021 Alin Popa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * \author Alin Popa <alin.popa@fxdata.ro>
 */

#include "ProcEntry.h"
#include "Application.h"

namespace tkm::monitor
{

ProcEntry::ProcEntry(int pid)
: m_pid(pid)
{
    m_timer = std::make_shared<Timer>("ProcEntry", [this]() {
        return (TaskMonitor()->getManager()->getNetLinkStats()->requestTaskAcct(m_pid) != -1)
                   ? true
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
