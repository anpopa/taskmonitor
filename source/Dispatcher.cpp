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

#include "Registry.h"
#include "SysProcMeminfo.h"
#include "SysProcPressure.h"
#include "SysProcStat.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

static bool doGetProcAcct(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq);
static bool doGetProcEventStats(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq);
static bool doGetSysProcMeminfo(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq);
static bool doGetSysProcStat(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq);
static bool doGetSysProcPsi(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq);

Dispatcher::Dispatcher(const shared_ptr<Options> options)
: m_options(options)
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
  case Dispatcher::Action::GetProcAcct:
    return doGetProcAcct(getShared(), request);
  case Dispatcher::Action::GetProcEventStats:
    return doGetProcEventStats(getShared(), request);
  case Dispatcher::Action::GetSysProcMeminfo:
    return doGetSysProcMeminfo(getShared(), request);
  case Dispatcher::Action::GetSysProcStat:
    return doGetSysProcStat(getShared(), request);
  case Dispatcher::Action::GetSysProcPressure:
    return doGetSysProcPsi(getShared(), request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doGetProcAcct(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq)
{
  Registry::Request regrq = {.action = Registry::Action::CollectAndSend, .collector = rq.collector};
  return App()->getRegistry()->pushRequest(regrq);
}

static bool doGetProcEventStats(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq)
{
  ProcEvent::Request regrq = {.action = ProcEvent::Action::CollectAndSend,
                              .collector = rq.collector};
  return App()->getProcEvent()->pushRequest(regrq);
}

static bool doGetSysProcMeminfo(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq)
{
  SysProcMeminfo::Request regrq = {.action = SysProcMeminfo::Action::CollectAndSend,
                                   .collector = rq.collector};
  return App()->getSysProcMeminfo()->pushRequest(regrq);
}

static bool doGetSysProcStat(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq)
{
  SysProcStat::Request regrq = {.action = SysProcStat::Action::CollectAndSend,
                                .collector = rq.collector};
  return App()->getSysProcStat()->pushRequest(regrq);
}

static bool doGetSysProcPsi(const shared_ptr<Dispatcher> disp, const Dispatcher::Request &rq)
{
  SysProcPressure::Request regrq = {.action = SysProcPressure::Action::CollectAndSend,
                                    .collector = rq.collector};
  return App()->getSysProcPressure()->pushRequest(regrq);
}

} // namespace tkm::monitor
