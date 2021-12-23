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

#include "SysProcStat.h"
#include "Application.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static constexpr int statCpuNamePos = 0;
static constexpr int statUserJiffiesPos = 1;
static constexpr int statSystemJiffiesPos = 3;

namespace tkm::monitor
{

void CPUStat::updateStats(uint64_t newUserJiffies, uint64_t newSystemJiffies)
{
    if (m_lastUserJiffies == 0) {
        m_lastUserJiffies = newUserJiffies;
    }

    if (m_lastSystemJiffies == 0) {
        m_lastSystemJiffies = newSystemJiffies;
    }

    auto userJiffiesDiff
        = ((newUserJiffies - m_lastUserJiffies) > 0) ? (newUserJiffies - m_lastUserJiffies) : 0;
    auto sysJiffiesDiff = ((newSystemJiffies - m_lastSystemJiffies) > 0)
                              ? (newSystemJiffies - m_lastSystemJiffies)
                              : 0;

    m_lastUserJiffies = newUserJiffies;
    m_lastSystemJiffies = newSystemJiffies;

    m_userPercent = jiffiesToPercent(userJiffiesDiff);
    m_sysPercent = jiffiesToPercent(sysJiffiesDiff);
    m_totalPercent = jiffiesToPercent(userJiffiesDiff + sysJiffiesDiff);
}

void CPUStat::printStats(void)
{
    logInfo() << "MON::SYS::STAT[" << m_name << "] "
              << "Total=" << m_totalPercent << "% "
              << "User=" << m_userPercent << "% "
              << "System=" << m_sysPercent << "%";
}

SysProcStat::SysProcStat(std::shared_ptr<Options> &options)
: m_options(options)
{
    try {
        m_usecInterval = std::stol(m_options->getFor(Options::Key::StatPollInterval));
    } catch (...) {
        throw std::runtime_error("Fail process StatPollInterval");
    }

    m_file = std::make_unique<std::ifstream>("/proc/stat");
    if (!m_file->is_open()) {
        throw std::runtime_error("Fail to open stat file");
    }

    m_timer = std::make_shared<Timer>("SysProcStat", [this]() { return processOnTick(); });
}

void SysProcStat::startMonitoring(void)
{
    m_timer->start(m_usecInterval, true);
    TaskMonitor()->addEventSource(m_timer);
}

void SysProcStat::disable(void)
{
    m_timer->stop();
    TaskMonitor()->remEventSource(m_timer);
}

bool SysProcStat::processOnTick(void)
{
    m_file->seekg(0, std::ios::beg);

    std::string line;
    while (std::getline(*m_file, line)) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string buf;

        if (line.find("cpu") == std::string::npos) {
            break;
        }

        while (ss >> buf) {
            tokens.push_back(buf);
        }

        if (tokens.size() < 4) {
            logError() << "Proc stat file parse error";
            return false;
        }

        auto updateCpuStatEntry = [this, tokens](const std::shared_ptr<CPUStat> &entry) {
            uint64_t newUserJiffies = 0;
            uint64_t newSysJiffies = 0;

            try {
                newUserJiffies = std::stoul(tokens[statUserJiffiesPos].c_str());
                newSysJiffies = std::stoul(tokens[statSystemJiffiesPos].c_str());
            } catch (...) {
                logError() << "Cannot convert stat data to Jiffies";
                return;
            }

            entry->updateStats(newUserJiffies, newSysJiffies);
            entry->printStats();
        };

        auto found = false;
        m_cpus.foreach (
            [this, tokens, &found, updateCpuStatEntry](const std::shared_ptr<CPUStat> &entry) {
                if (entry->getName() == tokens[statCpuNamePos].c_str()) {
                    updateCpuStatEntry(entry);
                    found = true;
                }
            });

        if (!found) {
            logInfo() << "Adding new cpu core for statistics";

            std::shared_ptr<CPUStat> entry
                = std::make_shared<CPUStat>(tokens[statCpuNamePos].c_str(), m_usecInterval);

            updateCpuStatEntry(entry);
            m_cpus.append(entry);
            m_cpus.commit();
        }
    }

    return true;
}

} // namespace tkm::monitor