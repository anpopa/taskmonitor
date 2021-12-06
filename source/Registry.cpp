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

#include "Registry.h"
#include "Application.h"

#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

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
            logDebug() << "Add process monitoring for pid " << pid;
            std::shared_ptr<ProcEntry> entry = std::make_shared<ProcEntry>(pid);
            entry->startMonitoring(3000000);
            m_list.append(entry);
        }
    }

    // Commit our updated list
    m_list.commit();
}

void Registry::addEntry(int pid)
{
    auto found = false;

    m_list.foreach ([this, &found, pid](const std::shared_ptr<ProcEntry> &entry) {
        if (entry->getPid() == pid) {
            found = true;
        }
    });

    if (!found) {
        std::shared_ptr<ProcEntry> entry = std::make_shared<ProcEntry>(pid);
        entry->startMonitoring(3000000);
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

} // namespace tkm::monitor