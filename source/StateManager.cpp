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
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include <fstream>
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
static bool doUpdateProcessList(void);

#ifdef WITH_WAKE_LOCK
static const std::string gWakeLockName{"taskmonitor"};
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

  if (fs::exists("/sys/power/wake_unlock")) {
    std::ofstream fLock("/sys/power/wake_unlock");
    fLock << gWakeLockName;
    logInfo() << "Remove any existing taskmonitor wake locks";
  }
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
  } else {
    App()->remEventSource(m_queue);
    App()->remEventSource(m_collectorsTimer);
  }
}

auto StateManager::requestHandler(const StateManager::Request &request) -> bool
{
  switch (request.action) {
  case StateManager::Action::MonitorCollector:
    return doMonitorCollector(getShared(), request);
  case StateManager::Action::RemoveCollector:
    return doRemoveCollector(getShared(), request);
  case StateManager::Action::UpdateWakeLock:
    return doUpdateWakeLock(getShared());
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

    if (haveTcpCollector && !gActiveWakeLock) {
      if (fs::exists("/sys/power/wake_lock")) {
        std::ofstream fLock("/sys/power/wake_lock");
        fLock << gWakeLockName;
        gActiveWakeLock = true;
        logInfo() << "TCP collectors wake lock enabled";
      } else {
        logWarn() << "Wake lock sysfs interface not available";
      }
    }

    if (!haveTcpCollector && gActiveWakeLock) {
      if (fs::exists("/sys/power/wake_unlock")) {
        std::ofstream fLock("/sys/power/wake_unlock");
        fLock << gWakeLockName;
        gActiveWakeLock = false;
        logInfo() << "TCP collectors wake lock disabled";
      } else {
        logWarn() << "Wake unlock sysfs interface not available";
      }
    }
  }
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
