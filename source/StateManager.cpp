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

#ifdef WITH_WAKE_LOCK
#include <fstream>
#include <sdbus-c++/IProxy.h>
#endif

#include "Application.h"
#include "ICollector.h"
#include "StateManager.h"

namespace tkm::monitor
{

static bool doMonitorCollector(const std::shared_ptr<StateManager> mgr,
                               const StateManager::Request &rq);
static bool doRemoveCollector(const std::shared_ptr<StateManager> mgr,
                              const StateManager::Request &rq);
static bool doUpdateWakeLock(const std::shared_ptr<StateManager> mgr);
static bool doResetBSHPowerManagerProxy(const std::shared_ptr<StateManager> mgr);
static bool doUpdateProcessList(void);

#ifdef WITH_WAKE_LOCK
static const std::string gWakeLockName{"TaskMonitor"};
static bool gActiveWakeLock = false;
#endif

StateManager::StateManager(const std::shared_ptr<Options> options)
: m_options(options)
{
  const auto collectorTimeout =
      std::stoul(m_options->getFor(Options::Key::CollectorInactiveTimeout));

  m_queue = std::make_shared<AsyncQueue<Request>>(
      "StateManagerEventQueue", [this](const Request &request) { return requestHandler(request); });

  m_collectorsTimer = std::make_shared<Timer>("CollectorsStateTimer", [this, collectorTimeout]() {
    const auto timeNow = std::chrono::steady_clock::now();
    using USec = std::chrono::microseconds;

    m_activeCollectorList.foreach (
        [&timeNow, collectorTimeout](const std::shared_ptr<ICollector> &entry) {
          auto durationUs =
              std::chrono::duration_cast<USec>(timeNow - entry->getLastUpdateTime()).count();

          if (durationUs > collectorTimeout) {
            const std::map<Defaults::Arg, std::string> reqArgs{
                std::make_pair<Defaults::Arg, std::string>(
                    Defaults::Arg::WithEventSource,
                    std::string(tkmDefaults.argFor(Defaults::Arg::WithEventSource)))};

            logWarn() << "Collector " << entry->getName()
                      << " is inactive. Remove collector connection";
            StateManager::Request rq = {.action = StateManager::Action::RemoveCollector,
                                        .collector = entry,
                                        .args = std::move(reqArgs)};
            App()->getStateManager()->pushRequest(rq);
          }
        });

    return true;
  });
  m_collectorsTimer->start(collectorTimeout, true);

#ifdef WITH_WAKE_LOCK
  resetBSHPowerManagerProxy();
  m_wakeLockRefreshTimer = std::make_shared<Timer>("WakeLockRefreshTimer", [this]() {
    if (gActiveWakeLock) {
      bool haveTcpCollector = false;
      m_activeCollectorList.foreach ([&haveTcpCollector](const std::shared_ptr<ICollector> &entry) {
        if (entry->getType() == ICollector::Type::TCP) {
          haveTcpCollector = true;
        }
      });

      if (haveTcpCollector && (getBSHPowerManagerProxy() != nullptr)) {
        int32_t result = 0;
        try {
          getBSHPowerManagerProxy()
              ->callMethod("LockSuspend")
              .onInterface("com.bshg.PowerManager")
              .withArguments(gWakeLockName)
              .storeResultsTo(result);
        } catch (std::exception &e) {
          logError() << "Fail to LockSuspend. Exception: " << e.what();
          StateManager::Request rq = {.action = StateManager::Action::ResetBSHPowerManagerProxy};
          pushRequest(rq);
          result = -1;
        }

        if (result != 0) {
          logWarn() << "Failed to refresh lock suspend";
        } else {
          logInfo() << "TCP collectors wake lock refreshed";
        }
      }
    }

    return true;
  });
  m_wakeLockRefreshTimer->start(10000000, true); // refresh every 10sec
#endif
}

auto StateManager::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void StateManager::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(m_queue);
    App()->addEventSource(m_collectorsTimer);
    if (m_wakeLockRefreshTimer != nullptr) {
      App()->addEventSource(m_wakeLockRefreshTimer);
    }
  } else {
    App()->remEventSource(m_queue);
    App()->remEventSource(m_collectorsTimer);
    if (m_wakeLockRefreshTimer != nullptr) {
      App()->remEventSource(m_wakeLockRefreshTimer);
    }
  }
}

#ifdef WITH_WAKE_LOCK
void StateManager::resetBSHPowerManagerProxy(void)
{
  constexpr const char *busName = "com.bshg.powermanager";
  constexpr const char *objectPath = "/com/bshg/PowerManager";

  try {
    m_bshPowerManagerProxy = sdbus::createProxy(busName, objectPath);
  } catch (std::exception &e) {
    logError() << "Fail to create BSH PM Proxy. Exception: " << e.what();
    StateManager::Request rq = {.action = StateManager::Action::ResetBSHPowerManagerProxy};
    pushRequest(rq);
  }
}
#endif

auto StateManager::requestHandler(const StateManager::Request &request) -> bool
{
  switch (request.action) {
  case StateManager::Action::MonitorCollector:
    return doMonitorCollector(getShared(), request);
  case StateManager::Action::RemoveCollector:
    return doRemoveCollector(getShared(), request);
  case StateManager::Action::UpdateWakeLock:
    return doUpdateWakeLock(getShared());
  case StateManager::Action::ResetBSHPowerManagerProxy:
    return doResetBSHPowerManagerProxy(getShared());
  case StateManager::Action::UpdateProcessList:
    return doUpdateProcessList();
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doMonitorCollector(const std::shared_ptr<StateManager> mgr,
                               const StateManager::Request &rq)
{
  mgr->getActiveCollectorList().append(rq.collector, true);

  // Update wake lock
  if (rq.collector->getType() == ICollector::Type::TCP) {
    StateManager::Request wrq = {.action = StateManager::Action::UpdateWakeLock,
                                 .collector = rq.collector};
    mgr->pushRequest(wrq);
  }
  return true;
}

static bool doRemoveCollector(const std::shared_ptr<StateManager> mgr,
                              const StateManager::Request &rq)
{
  // Remove the event source if requested
  if (rq.args.count(Defaults::Arg::WithEventSource)) {
    App()->remEventSource(rq.collector);
  }

  // Remove our reference
  mgr->getActiveCollectorList().remove(rq.collector, true);

  // Update wake lock
  if (rq.collector->getType() == ICollector::Type::TCP) {
    StateManager::Request wrq = {.action = StateManager::Action::UpdateWakeLock,
                                 .collector = rq.collector};
    mgr->pushRequest(wrq);
  }

  return true;
}

static bool doUpdateWakeLock(const std::shared_ptr<StateManager> mgr)
{
#ifdef WITH_WAKE_LOCK
  if (App()->getOptions()->getFor(Options::Key::TCPActiveWakeLock) ==
      tkmDefaults.valFor(Defaults::Val::True)) {
    bool haveTcpCollector = false;

    mgr->getActiveCollectorList().foreach (
        [&haveTcpCollector](const std::shared_ptr<ICollector> &entry) {
          if (entry->getType() == ICollector::Type::TCP) {
            haveTcpCollector = true;
          }
        });

    if (haveTcpCollector && !gActiveWakeLock && (mgr->getBSHPowerManagerProxy() != nullptr)) {
      int32_t result = 0;
      try {
        mgr->getBSHPowerManagerProxy()
            ->callMethod("LockSuspend")
            .onInterface("com.bshg.PowerManager")
            .withArguments(gWakeLockName)
            .storeResultsTo(result);
      } catch (std::exception &e) {
        logError() << "Fail to LockSuspend. Exception: " << e.what();
        StateManager::Request rq = {.action = StateManager::Action::ResetBSHPowerManagerProxy};
        mgr->pushRequest(rq);
        result = -1;
      }
      if (result != 0) {
        logWarn() << "Failed to lock suspend";
      } else {
        logInfo() << "TCP collectors wake lock enabled";
        gActiveWakeLock = true;
      }
    }

    if (!haveTcpCollector && gActiveWakeLock && (mgr->getBSHPowerManagerProxy() != nullptr)) {
      int32_t result = 0;
      try {
        mgr->getBSHPowerManagerProxy()
            ->callMethod("UnlockSuspend")
            .onInterface("com.bshg.PowerManager")
            .withArguments(gWakeLockName)
            .storeResultsTo(result);
      } catch (std::exception &e) {
        logError() << "Fail to UnlockSuspend. Exception: " << e.what();
        StateManager::Request rq = {.action = StateManager::Action::ResetBSHPowerManagerProxy};
        mgr->pushRequest(rq);
        result = -1;
      }
      if (result != 0) {
        logWarn() << "Failed to unlock suspend";
      } else {
        logInfo() << "TCP collectors wake lock disabled";
        gActiveWakeLock = false;
      }
    }
  }
#endif
  return true;
}

static bool doResetBSHPowerManagerProxy(const std::shared_ptr<StateManager> mgr)
{
#ifdef WITH_WAKE_LOCK
  mgr->resetBSHPowerManagerProxy();
#endif
  return true;
}

static bool doUpdateProcessList(void)
{
  if (App()->getProcRegistry() != nullptr) {
    App()->getProcRegistry()->updateProcessList();
  }
  return true;
}

} // namespace tkm::monitor
