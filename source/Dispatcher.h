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

#pragma once

#include <map>
#include <string>

#include "Options.h"
#include "ProcAcct.h"
#include "ProcEvent.h"

#include "ICollector.h"

#include "../bswinfra/source/AsyncQueue.h"

using namespace bswi::event;

namespace tkm::monitor
{

class Dispatcher : public std::enable_shared_from_this<Dispatcher>
{
public:
  enum class Action {
    GetStartupData,
    GetProcAcct,
    GetProcInfo,
    GetProcEventStats,
    GetSysProcMemInfo,
    GetSysProcDiskStats,
    GetSysProcStat,
    GetSysProcPressure,
    GetSysProcBuddyInfo,
    GetSysProcWireless,
    GetContextInfo
  };

  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
    std::map<std::string, std::string> args;
  } Request;

public:
  explicit Dispatcher(const std::shared_ptr<Options> options);

public:
  auto getShared() -> std::shared_ptr<Dispatcher> { return shared_from_this(); }
  auto pushRequest(Request &request) -> int;
  void enableEvents();

private:
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
};

} // namespace tkm::monitor
