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
#include "../bswinfra/source/SafeList.h"
#include "../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{
struct PressureStat : public std::enable_shared_from_this<PressureStat> {
public:
    explicit PressureStat(const std::string &name)
    : m_name(name) {};
    ~PressureStat() = default;

public:
    PressureStat(PressureStat const &) = delete;
    void operator=(PressureStat const &) = delete;

    auto getName(void) -> const std::string & { return m_name; }
    void printStats(void);

private:
    std::string m_name;
};

class SysProcPressure : public std::enable_shared_from_this<SysProcPressure>
{
public:
    explicit SysProcPressure(std::shared_ptr<Options> &options);
    ~SysProcPressure() = default;

public:
    SysProcPressure(SysProcPressure const &) = delete;
    void operator=(SysProcPressure const &) = delete;

public:
    auto getShared() -> std::shared_ptr<SysProcPressure> { return shared_from_this(); }
    void enableEvents();

    void startMonitoring(void);
    void disable(void);

private:
    bool processOnTick(void);

private:
    bswi::util::SafeList<std::shared_ptr<PressureStat>> m_entries {"StatPressureList"};
    std::shared_ptr<Options> m_options = nullptr;
    std::shared_ptr<Timer> m_timer = nullptr;
    size_t m_usecInterval = 0;
};

} // namespace tkm::monitor