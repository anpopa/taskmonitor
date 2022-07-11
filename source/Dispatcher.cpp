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

#include "ProcRegistry.h"
#include "SysProcDiskStats.h"
#include "SysProcMemInfo.h"
#include "SysProcPressure.h"
#include "SysProcStat.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

static bool doGetProcAcct(const Dispatcher::Request &rq);
static bool doGetProcInfo(const Dispatcher::Request &rq);
static bool doGetProcEventStats(const Dispatcher::Request &rq);
static bool doGetSysProcMemInfo(const Dispatcher::Request &rq);
static bool doGetSysProcDiskStats(const Dispatcher::Request &rq);
static bool doGetSysProcStat(const Dispatcher::Request &rq);
static bool doGetSysProcPsi(const Dispatcher::Request &rq);
static bool doGetSysProcBuddyInfo(const Dispatcher::Request &rq);
static bool doGetContextInfo(const Dispatcher::Request &rq);

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
    return doGetProcAcct(request);
  case Dispatcher::Action::GetProcInfo:
    return doGetProcInfo(request);
  case Dispatcher::Action::GetProcEventStats:
    return doGetProcEventStats(request);
  case Dispatcher::Action::GetSysProcMemInfo:
    return doGetSysProcMemInfo(request);
  case Dispatcher::Action::GetSysProcDiskStats:
    return doGetSysProcDiskStats(request);
  case Dispatcher::Action::GetSysProcStat:
    return doGetSysProcStat(request);
  case Dispatcher::Action::GetSysProcPressure:
    return doGetSysProcPsi(request);
  case Dispatcher::Action::GetSysProcBuddyInfo:
    return doGetSysProcBuddyInfo(request);
  case Dispatcher::Action::GetContextInfo:
    return doGetContextInfo(request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doGetProcAcct(const Dispatcher::Request &rq)
{
  ProcRegistry::Request regrq = {.action = ProcRegistry::Action::CollectAndSendProcAcct,
                                 .collector = rq.collector};
  return App()->getProcRegistry()->pushRequest(regrq);
}

static bool doGetProcInfo(const Dispatcher::Request &rq)
{
  ProcRegistry::Request regrq = {.action = ProcRegistry::Action::CollectAndSendProcInfo,
                                 .collector = rq.collector};
  return App()->getProcRegistry()->pushRequest(regrq);
}

static bool doGetContextInfo(const Dispatcher::Request &rq)
{
  ProcRegistry::Request regrq = {.action = ProcRegistry::Action::CollectAndSendContextInfo,
                                 .collector = rq.collector};
  return App()->getProcRegistry()->pushRequest(regrq);
}

static bool doGetProcEventStats(const Dispatcher::Request &rq)
{
  ProcEvent::Request regrq = {.action = ProcEvent::Action::CollectAndSend,
                              .collector = rq.collector};
  return App()->getProcEvent()->pushRequest(regrq);
}

static bool doGetSysProcMemInfo(const Dispatcher::Request &rq)
{
  // Ignore requests if the module is not enabled
  if (App()->getSysProcMemInfo() == nullptr) {
    return true;
  }
  SysProcMemInfo::Request regrq = {.action = SysProcMemInfo::Action::CollectAndSend,
                                   .collector = rq.collector};
  return App()->getSysProcMemInfo()->pushRequest(regrq);
}

static bool doGetSysProcDiskStats(const Dispatcher::Request &rq)
{
  // Ignore requests if the module is not enabled
  if (App()->getSysProcDiskStats() == nullptr) {
    return true;
  }
  SysProcDiskStats::Request regrq = {.action = SysProcDiskStats::Action::CollectAndSend,
                                     .collector = rq.collector};
  return App()->getSysProcDiskStats()->pushRequest(regrq);
}

static bool doGetSysProcStat(const Dispatcher::Request &rq)
{
  // Ignore requests if the module is not enabled
  if (App()->getSysProcStat() == nullptr) {
    return true;
  }
  SysProcStat::Request regrq = {.action = SysProcStat::Action::CollectAndSend,
                                .collector = rq.collector};
  return App()->getSysProcStat()->pushRequest(regrq);
}

static bool doGetSysProcBuddyInfo(const Dispatcher::Request &rq)
{
  // Ignore requests if the module is not enabled
  if (App()->getSysProcBuddyInfo() == nullptr) {
    return true;
  }
  SysProcBuddyInfo::Request regrq = {.action = SysProcBuddyInfo::Action::CollectAndSend,
                                     .collector = rq.collector};
  return App()->getSysProcBuddyInfo()->pushRequest(regrq);
}

static bool doGetSysProcPsi(const Dispatcher::Request &rq)
{
  // Ignore requests if the module is not enabled
  if (App()->getSysProcPressure() == nullptr) {
    return true;
  }
  SysProcPressure::Request regrq = {.action = SysProcPressure::Action::CollectAndSend,
                                    .collector = rq.collector};
  return App()->getSysProcPressure()->pushRequest(regrq);
}

} // namespace tkm::monitor
