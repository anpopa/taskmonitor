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

#include <time.h>
#include <unistd.h>

#include "Options.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/IApplication.h"
#include "../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{

struct CPUStat : public std::enable_shared_from_this<CPUStat> {
public:
    explicit CPUStat(const std::string &name, size_t usecInterval)
    : m_name(name)
    , m_usecInterval(usecInterval)
    {
        m_sysHZ = sysconf(_SC_CLK_TCK);
    };
    ~CPUStat() = default;

public:
    CPUStat(CPUStat const &) = delete;
    void operator=(CPUStat const &) = delete;

    auto getName(void) -> const std::string & { return m_name; }

    void updateStats(uint64_t newUserJiffies, uint64_t newSystemJiffies);
    void printStats(void);

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
    std::string m_name;
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

private:
    bool processOnTick(void);

private:
    bswi::util::SafeList<std::shared_ptr<CPUStat>> m_cpus {"StatCPUList"};
    std::unique_ptr<std::ifstream> m_file = nullptr;
    std::shared_ptr<Options> m_options = nullptr;
    std::shared_ptr<Timer> m_timer = nullptr;
    size_t m_usecInterval = 0;
};

} // namespace tkm::monitor