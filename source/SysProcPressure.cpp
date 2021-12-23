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

#include "SysProcPressure.h"
#include "Application.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace tkm::monitor
{

void PressureStat::printStats(void)
{
    std::ifstream file("/proc/pressure/" + m_name);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            logInfo() << "MON::STAT::PRESSURE[" << m_name << "] " << line;
        }
    } else {
        logError() << "Cannot open pressure file: "
                   << "/proc/pressure/" + m_name;
    }
}

SysProcPressure::SysProcPressure(std::shared_ptr<Options> &options)
: m_options(options)
{
    try {
        m_usecInterval = std::stol(m_options->getFor(Options::Key::PressurePollInterval));
    } catch (...) {
        throw std::runtime_error("Fail process pressure PollInterval");
    }

    if (options->getFor(Options::Key::PressureWithCPU) == "true") {
        std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("cpu");
        m_entries.append(entry);
    }

    if (options->getFor(Options::Key::PressureWithMemory) == "true") {
        std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("memory");
        m_entries.append(entry);
    }

    if (options->getFor(Options::Key::PressureWithIO) == "true") {
        std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("io");
        m_entries.append(entry);
    }

    m_entries.commit();
    m_timer = std::make_shared<Timer>("SysProcPressure", [this]() { return processOnTick(); });
}

void SysProcPressure::startMonitoring(void)
{
    m_timer->start(m_usecInterval, true);
    TaskMonitor()->addEventSource(m_timer);
}

void SysProcPressure::disable(void)
{
    m_timer->stop();
    TaskMonitor()->remEventSource(m_timer);
}

bool SysProcPressure::processOnTick(void)
{
    m_entries.foreach ([this](const std::shared_ptr<PressureStat> &entry) { entry->printStats(); });

    return true;
}

} // namespace tkm::monitor