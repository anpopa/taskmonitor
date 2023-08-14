/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2023
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     StateManager Class
 * @details   Manage and update internal states of the service
 *-
 */

#pragma once

#include <sdbus-c++/IProxy.h>
#include <string>

#include "ICollector.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"
#include "../bswinfra/source/Timer.h"

#ifdef WITH_WAKE_LOCK
#include <sdbus-c++/sdbus-c++.h>
#endif

using namespace bswi::event;

namespace tkm::monitor
{

class StateManager : public std::enable_shared_from_this<StateManager>
{
public:
  enum class Action {
    MonitorCollector,
    RemoveCollector,
    UpdateWakeLock,
    UpdateProcessList,
    ResetBSHPowerManagerProxy
  };

  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
    std::map<Defaults::Arg, std::string> args;
  } Request;

public:
  explicit StateManager(const std::shared_ptr<Options> options);
  virtual ~StateManager() = default;

public:
  StateManager(StateManager const &) = delete;
  void operator=(StateManager const &) = delete;

public:
  auto getShared() -> std::shared_ptr<StateManager> { return shared_from_this(); }
  void setEventSource(bool enabled = true);

  auto getActiveCollectorList(void) -> bswi::util::SafeList<std::shared_ptr<ICollector>> &
  {
    return m_activeCollectorList;
  }
#ifdef WITH_WAKE_LOCK
  auto getBSHPowerManagerProxy(void) -> std::shared_ptr<sdbus::IProxy>
  {
    return m_bshPowerManagerProxy;
  }
  void resetBSHPowerManagerProxy(void);
#endif

  auto pushRequest(StateManager::Request &request) -> int;

private:
  bool requestHandler(const Request &request);

private:
  bswi::util::SafeList<std::shared_ptr<ICollector>> m_activeCollectorList{"ActiveCollectorList"};
#ifdef WITH_WAKE_LOCK
  std::shared_ptr<sdbus::IProxy> m_bshPowerManagerProxy = nullptr;
#endif
  std::shared_ptr<Timer> m_wakeLockRefreshTimer = nullptr;
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Timer> m_collectorsTimer = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
};

} // namespace tkm::monitor
