/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcPresure Class
 * @details   Collect and report PSI information
 *-
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

std::vector<std::string> tokenize(const std::string &str, const char &ch)
{
    std::string next;
    std::vector<std::string> result;

    for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
        if (*it == ch) {
            if (!next.empty()) {
                result.push_back(next);
                next.clear();
            }
        } else {
            next += *it;
        }
    }

    if (!next.empty())
        result.push_back(next);

    return result;
}

void PressureStat::updateStats(void)
{
    std::ifstream file("/proc/pressure/" + m_name);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> tokens;
            std::stringstream ss(line);
            std::string buf;

            while (ss >> buf) {
                tokens.push_back(buf);
            }

            Json::Value data;
            for (size_t i = 1; i < tokens.size(); i++) {
                std::vector<std::string> keyVal = tokenize(tokens[i], '=');

                if (keyVal.size() == 2) {
                    data[keyVal[0]] = keyVal[1];
                }
            }

            m_jsonData[tokens[0]] = data;
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
    Json::Value head;
    head["type"] = "psi";
    head["time"] = time(NULL);

    m_entries.foreach ([this, &head](const std::shared_ptr<PressureStat> &entry) {
        entry->updateStats();
        head[entry->getName()] = entry->getJsonData();
    });

    writeJsonStream() << head;

    return true;
}

} // namespace tkm::monitor