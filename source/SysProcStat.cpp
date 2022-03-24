/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcStats Class
 * @details   Collect and report information from /proc/stats
 *-
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

auto SysProcStat::getCPUStat(const std::string &name) -> const std::shared_ptr<CPUStat>
{
    std::shared_ptr<CPUStat> cpuStat = nullptr;

    m_cpus.foreach ([this, &name, &cpuStat](const std::shared_ptr<CPUStat> &entry) {
        if (entry->getName() == name) {
            cpuStat = entry;
        }
    });

    return cpuStat;
}

} // namespace tkm::monitor