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

static bool doGetProcAcct(const shared_ptr<Dispatcher> &dispatcher,
                          const Dispatcher::Request &request);
static bool doGetProcEventStats(const shared_ptr<Dispatcher> &dispatcher,
                                const Dispatcher::Request &request);
static bool doGetSysProcMeminfo(const shared_ptr<Dispatcher> &dispatcher,
                                const Dispatcher::Request &request);
static bool doGetSysProcStat(const shared_ptr<Dispatcher> &dispatcher,
                             const Dispatcher::Request &request);
static bool doGetSysProcPressure(const shared_ptr<Dispatcher> &dispatcher,
                                 const Dispatcher::Request &request);

Dispatcher::Dispatcher(shared_ptr<Options> &options)
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
    return doGetSysProcPressure(getShared(), request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doGetProcAcct(const shared_ptr<Dispatcher> &dispatcher,
                          const Dispatcher::Request &request)
{
  Registry::Request rq = {.action = Registry::Action::CollectAndSend,
                          .collector = request.collector};
  return App()->getRegistry()->pushRequest(rq);
}

static bool doGetProcEventStats(const shared_ptr<Dispatcher> &dispatcher,
                                const Dispatcher::Request &request)
{
  ProcEvent::Request rq = {.action = ProcEvent::Action::CollectAndSend,
                           .collector = request.collector};
  return App()->getProcEvent()->pushRequest(rq);
}

static bool doGetSysProcMeminfo(const shared_ptr<Dispatcher> &dispatcher,
                                const Dispatcher::Request &request)
{
  SysProcMeminfo::Request rq = {.action = SysProcMeminfo::Action::CollectAndSend,
                                .collector = request.collector};
  return App()->getSysProcMeminfo()->pushRequest(rq);
}

static bool doGetSysProcStat(const shared_ptr<Dispatcher> &dispatcher,
                             const Dispatcher::Request &request)
{
  SysProcStat::Request rq = {.action = SysProcStat::Action::CollectAndSend,
                             .collector = request.collector};
  return App()->getSysProcStat()->pushRequest(rq);
}

static bool doGetSysProcPressure(const shared_ptr<Dispatcher> &dispatcher,
                                 const Dispatcher::Request &request)
{
  SysProcPressure::Request rq = {.action = SysProcPressure::Action::CollectAndSend,
                                 .collector = request.collector};
  return App()->getSysProcPressure()->pushRequest(rq);
}

} // namespace tkm::monitor
