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

#pragma once

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
    void startMonitoring(size_t interval);
    void disable(void);

    void setLastUserCPUTime(uint64_t cpuTime) { m_lastUserCPUTime = cpuTime; }
    void setLastSystemCPUTime(uint64_t cpuTime) { m_lastSystemCPUTime = cpuTime; }
    auto getLastUserCPUTime(void) -> uint64_t { return m_lastUserCPUTime; }
    auto getLastSystemCPUTime(void) -> uint64_t { return m_lastSystemCPUTime; }

private:
    std::shared_ptr<Timer> m_timer = nullptr;
    uint64_t m_lastUserCPUTime = 0;
    uint64_t m_lastSystemCPUTime = 0;
    int m_pid = 0;
};

} // namespace tkm::monitor
