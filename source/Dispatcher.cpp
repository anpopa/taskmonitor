/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Dispatcher Class
 * @details   Application dispatcher manager
 *-
 */

#include <filesystem>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "Dispatcher.h"
#include "ProcEntry.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

static bool doActionRegisterEvents(Dispatcher *manager, const Dispatcher::Request &request);

Dispatcher::Dispatcher(shared_ptr<Options> &options,
                       shared_ptr<ProcAcct> &procAcct,
                       shared_ptr<ProcEvent> &procEvent)
: m_options(options)
, m_procAcct(procAcct)
, m_procEvent(procEvent)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "DispatcherQueue", [this](const Request &request) { return requestHandler(request); });
}

auto Dispatcher::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void Dispatcher::enableEvents()
{
  App()->addEventSource(m_queue);
}

auto Dispatcher::requestHandler(const Request &request) -> bool
{
  switch (request.action) {
  case Dispatcher::Action::RegisterEvents:
    return doActionRegisterEvents(this, request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doActionRegisterEvents(Dispatcher *manager, const Dispatcher::Request &)
{
  logDebug() << "Opt proc at init " << App()->getOptions()->getFor(Options::Key::ReadProcAtInit);
  if (App()->getOptions()->getFor(Options::Key::ReadProcAtInit) == "true") {
    App()->getRegistry()->initFromProc();
  } else {
    App()->getRegistry()->addEntry(getpid());
  }

  if (App()->getOptions()->getFor(Options::Key::EnableSysStat) == "true") {
    App()->getSysProcStat()->startMonitoring();
  }

  if (App()->getOptions()->getFor(Options::Key::EnableSysMeminfo) == "true") {
    App()->getSysProcMeminfo()->startMonitoring();
  }

  if (App()->getOptions()->getFor(Options::Key::EnableSysPressure) == "true") {
    App()->getSysProcPressure()->startMonitoring();
  }

  // Start process monitoring
  manager->getProcEvent()->startProcMonitoring();

  return true;
}

} // namespace tkm::monitor
