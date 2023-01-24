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

#include "StateManager.h"
#include "Application.h"

namespace tkm::monitor
{

static bool doMonitorCollector(const std::shared_ptr<StateManager> mgr,
                               const StateManager::Request &rq);
static bool doRemoveCollector(const std::shared_ptr<StateManager> mgr,
                              const StateManager::Request &rq);

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

  return true;
}

} // namespace tkm::monitor
