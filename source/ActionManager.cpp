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

#include <filesystem>
#include <unistd.h>

#include "ActionManager.h"
#include "Application.h"
#include "Defaults.h"
#include "ProcEntry.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

static auto doActionRegisterEvents(ActionManager *manager, const ActionManager::Request &request)
    -> bool;

ActionManager::ActionManager(shared_ptr<Options> &options,
                             shared_ptr<NetLinkStats> &nlStats,
                             shared_ptr<NetLinkProc> &nlProc)
: m_options(options)
, m_nlStats(nlStats)
, m_nlProc(nlProc)
{
    m_queue = std::make_shared<AsyncQueue<Request>>(
        "ActionManagerQueue", [this](const Request &request) { return requestHandler(request); });
}

auto ActionManager::pushRequest(Request &request) -> int
{
    return m_queue->push(request);
}

void ActionManager::enableEvents()
{
    TaskMonitor()->addEventSource(m_queue);
}

auto ActionManager::requestHandler(const Request &request) -> bool
{
    switch (request.action) {
    case ActionManager::Action::RegisterEvents:
        return doActionRegisterEvents(this, request);
    default:
        break;
    }

    logError() << "Unknown action request";
    return false;
}

static auto doActionRegisterEvents(ActionManager *manager, const ActionManager::Request &) -> bool
{
    logDebug() << "Opt proc at init "
               << TaskMonitor()->getOptions()->getFor(Options::Key::ReadProcAtInit);
    if (TaskMonitor()->getOptions()->getFor(Options::Key::ReadProcAtInit) == "true") {
        // Read initial proc entries which includes ourselfs
        TaskMonitor()->getRegistry()->initFromProc();
    } else {
        // Add entry for ourselfs
        TaskMonitor()->getRegistry()->addEntry(getpid());
    }

    if (TaskMonitor()->getOptions()->getFor(Options::Key::EnableSysStat) == "true") {
        TaskMonitor()->getSysProcStat()->startMonitoring();
    }

    // Start process monitoring
    manager->getNetLinkProc()->startProcMonitoring();

    return true;
}

} // namespace tkm::monitor
