/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ActionManager Class
 * @details   Application action manager
 *-
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
        TaskMonitor()->getRegistry()->initFromProc();
    } else {
        TaskMonitor()->getRegistry()->addEntry(getpid());
    }

    if (TaskMonitor()->getOptions()->getFor(Options::Key::EnableSysStat) == "true") {
        TaskMonitor()->getSysProcStat()->startMonitoring();
    }

    if (TaskMonitor()->getOptions()->getFor(Options::Key::EnableSysPressure) == "true") {
        TaskMonitor()->getSysProcPressure()->startMonitoring();
    }

    // Start process monitoring
    manager->getNetLinkProc()->startProcMonitoring();

    return true;
}

} // namespace tkm::monitor
