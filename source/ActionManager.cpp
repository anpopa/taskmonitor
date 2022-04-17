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

static bool doActionRegisterEvents(ActionManager *manager, const ActionManager::Request &request);

ActionManager::ActionManager(shared_ptr<Options> &options,
                             shared_ptr<ProcAcct> &procAcct,
                             shared_ptr<ProcEvent> &procEvent)
: m_options(options)
, m_procAcct(procAcct)
, m_procEvent(procEvent)
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

static bool doActionRegisterEvents(ActionManager *manager, const ActionManager::Request &)
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

  if (TaskMonitor()->getOptions()->getFor(Options::Key::EnableSysMeminfo) == "true") {
    TaskMonitor()->getSysProcMeminfo()->startMonitoring();
  }

  if (TaskMonitor()->getOptions()->getFor(Options::Key::EnableSysPressure) == "true") {
    TaskMonitor()->getSysProcPressure()->startMonitoring();
  }

  // Start process monitoring
  manager->getProcEvent()->startProcMonitoring();

  return true;
}

} // namespace tkm::monitor
