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

#pragma once

#include <map>
#include <string>

#include "Options.h"
#include "ProcAcct.h"
#include "ProcEvent.h"

#include "../bswinfra/source/AsyncQueue.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ActionManager : public std::enable_shared_from_this<ActionManager>
{
public:
  enum class Action { RegisterEvents };

  typedef struct Request {
    Action action;
    std::map<std::string, std::string> args;
  } Request;

public:
  explicit ActionManager(std::shared_ptr<Options> &options,
                         std::shared_ptr<ProcAcct> &procAcct,
                         std::shared_ptr<ProcEvent> &procEvent);

public:
  auto getShared() -> std::shared_ptr<ActionManager> { return shared_from_this(); }
  void enableEvents();
  auto pushRequest(Request &request) -> int;
  auto getProcAcct() -> std::shared_ptr<ProcAcct> & { return m_procAcct; }
  auto getProcEvent() -> std::shared_ptr<ProcEvent> & { return m_procEvent; }

private:
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<Options> m_options = nullptr;
  std::shared_ptr<ProcAcct> m_procAcct = nullptr;
  std::shared_ptr<ProcEvent> m_procEvent = nullptr;
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
};

} // namespace tkm::monitor
