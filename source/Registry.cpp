/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Registry Class
 * @details   ProcEntry registry
 *-
 */

#include "Registry.h"
#include "Application.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>

#include "../bswinfra/source/KeyFile.h"

namespace fs = std::filesystem;
using namespace bswi::kf;

namespace tkm::monitor
{

void Registry::initFromProc(void)
{
    std::string path = "/proc";

    logDebug() << "Read existing proc entries";
    for (const auto &entry : fs::directory_iterator(path)) {
        int pid = -1;

        try {
            pid = std::stoi(entry.path().filename());
        } catch (...) {
            // Discard non pid entries
        }

        if (pid != -1) {
            if (!isBlacklisted(pid)) {
                logDebug() << "Add process monitoring for pid " << pid;
                std::shared_ptr<ProcEntry> entry = std::make_shared<ProcEntry>(pid);
                entry->startMonitoring(m_pollInterval);
                m_list.append(entry);
            }
        }
    }

    // Commit our updated list
    m_list.commit();
}

auto Registry::getEntry(int pid) -> const std::shared_ptr<ProcEntry>
{
    std::shared_ptr<ProcEntry> retEntry = nullptr;

    m_list.foreach ([this, pid, &retEntry](const std::shared_ptr<ProcEntry> &entry) {
        if (entry->getPid() == pid) {
            retEntry = entry;
        }
    });

    return retEntry;
}

void Registry::addEntry(int pid)
{
    auto found = false;

    m_list.foreach ([this, &found, pid](const std::shared_ptr<ProcEntry> &entry) {
        if (entry->getPid() == pid) {
            found = true;
        }
    });

    if (!found && !isBlacklisted(pid)) {
        std::shared_ptr<ProcEntry> entry = std::make_shared<ProcEntry>(pid);
        entry->startMonitoring(m_pollInterval);
        m_list.append(entry);
        m_list.commit();
    }
}

void Registry::remEntry(int pid)
{
    m_list.foreach ([this, pid](const std::shared_ptr<ProcEntry> &entry) {
        if (entry->getPid() == pid) {
            logDebug() << "Found entry to remove with pid " << pid;
            entry->disable();
            m_list.remove(entry);
        }
    });
    m_list.commit();
}

// If reading /proc/<pid>/stat fails or the proc name is in the blacklist
// we mark the pid as blacklisted by returning true
bool Registry::isBlacklisted(int pid)
{
    fs::path exePath {};

    try {
        exePath = fs::path("/proc") / fs::path(std::to_string(pid)) / fs::path("stat");
    } catch (const std::exception &e) {
        logError() << "Reading stat file for pid " << pid << " fails with exception " << e.what();
        return true;
    }

    if (fs::exists(exePath)) {
        std::unique_ptr<std::ifstream> statFile = nullptr;

        try {
            statFile = std::make_unique<std::ifstream>(exePath.string());
        } catch (const std::exception &e) {
            logError() << "Reading stat file for pid " << pid << " fails with exception "
                       << e.what();
            return true;
        }

        std::stringstream buffer;
        buffer << statFile->rdbuf();

        if (m_options->hasConfigFile()) {
            const std::vector<Property> props
                = m_options->getConfigFile()->getProperties("blacklist", -1);
            for (const auto &prop : props) {
                if (buffer.str().find(prop.key) != std::string::npos) {
                    return true;
                }
            }
        }
    }

    return false;
}

} // namespace tkm::monitor
