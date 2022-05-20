/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Application Class
 * @details   Main application class
 *-
 */

#include "Application.h"
#include "Defaults.h"
#include "ProcEntry.h"
#include <cstdint>
#include <memory>
#include <string>
#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif
#include <filesystem>

using std::shared_ptr;
using std::string;

#define USEC2SEC(x) (x / 1000000)

namespace tkm::monitor
{

Application *Application::appInstance = nullptr;

static bool shouldStartTCPServer(const std::shared_ptr<tkm::monitor::Options> opts);
static bool shouldStartUDSServer(const std::shared_ptr<tkm::monitor::Options> opts);

Application::Application(const string &name, const string &description, const string &configFile)
: bswi::app::IApplication(name, description)
{
  if (Application::appInstance != nullptr) {
    throw bswi::except::SingleInstance();
  }
  appInstance = this;

  m_options = std::make_shared<Options>(configFile);
  uint64_t fastLaneInterval, paceLaneInterval, slowLaneInterval;

  try {
    fastLaneInterval = std::stoul(m_options->getFor(Options::Key::FastLaneInterval));
  } catch (...) {
    fastLaneInterval = std::stoul(tkmDefaults.getFor(Defaults::Default::FastLaneInterval));
  }

  try {
    paceLaneInterval = std::stoul(m_options->getFor(Options::Key::PaceLaneInterval));
  } catch (...) {
    paceLaneInterval = std::stoul(tkmDefaults.getFor(Defaults::Default::PaceLaneInterval));
  }

  try {
    slowLaneInterval = std::stoul(m_options->getFor(Options::Key::SlowLaneInterval));
  } catch (...) {
    slowLaneInterval = std::stoul(tkmDefaults.getFor(Defaults::Default::SlowLaneInterval));
  }

  if (m_options->getFor(Options::Key::EnableTCPServer) == "true") {
    m_netServer = std::make_shared<TCPServer>(m_options);

    if (shouldStartTCPServer(m_options)) {
      try {
        m_netServer->bindAndListen();
      } catch (std::exception &e) {
        logError() << "Fail to start TCP server. Exception: " << e.what();
      }
    }
  }

  if (m_options->getFor(Options::Key::EnableUDSServer) == "true") {
    m_udsServer = std::make_shared<UDSServer>(m_options);

    if (shouldStartUDSServer(m_options)) {
      try {
        m_udsServer->start();
      } catch (std::exception &e) {
        logError() << "Fail to start UDS server. Exception: " << e.what();
      }
    }
  }

  m_procAcct = std::make_shared<ProcAcct>(m_options);
  m_procAcct->enableEvents();

  m_procEvent = std::make_shared<ProcEvent>(m_options);
  m_procEvent->enableEvents();

  m_registry = std::make_shared<Registry>(m_options);
  m_registry->enableEvents();

  m_sysProcStat = std::make_shared<SysProcStat>(m_options);
  m_sysProcStat->setUpdateInterval(fastLaneInterval);
  m_sysProcStat->enableEvents();

  m_sysProcMemInfo = std::make_shared<SysProcMemInfo>(m_options);
  m_sysProcStat->setUpdateInterval(paceLaneInterval);
  m_sysProcMemInfo->enableEvents();

  m_sysProcPressure = std::make_shared<SysProcPressure>(m_options);
  m_sysProcStat->setUpdateInterval(paceLaneInterval);
  m_sysProcPressure->enableEvents();

  m_sysProcDiskStats = std::make_shared<SysProcDiskStats>(m_options);
  m_sysProcStat->setUpdateInterval(slowLaneInterval);
  m_sysProcDiskStats->enableEvents();

  m_dispatcher = std::make_unique<Dispatcher>(m_options);
  m_dispatcher->enableEvents();

  enableUpdateLanes(fastLaneInterval, paceLaneInterval, slowLaneInterval);
  startWatchdog();
}

void Application::enableUpdateLanes(uint64_t fastLaneInterval,
                                    uint64_t paceLaneInterval,
                                    uint64_t slowLaneInterval)
{
  m_fastLaneTimer = std::make_shared<Timer>("FastLaneTimer", [this]() {
    // ProcInfo
    m_registry->getProcList().foreach (
        [](const std::shared_ptr<ProcEntry> &entry) { entry->updateProcInfo(); });
    // SysProcStat
    m_sysProcStat->update();

    return true;
  });

  m_paceLaneTimer = std::make_shared<Timer>("PaceLaneTimer", [this]() {
    // SysProcMemInfo
    m_sysProcMemInfo->update();
    // SysProcPressure
    m_sysProcPressure->update();

    return true;
  });

  m_slowLaneTimer = std::make_shared<Timer>("SlowLaneTimer", [this]() {
    // ProcAcct
    m_registry->getProcList().foreach (
        [](const std::shared_ptr<ProcEntry> &entry) { entry->updateProcAcct(); });
    // SysProcDiskStats
    m_sysProcDiskStats->update();

    return true;
  });

  m_fastLaneTimer->start(fastLaneInterval, true);
  m_paceLaneTimer->start(paceLaneInterval, true);
  m_slowLaneTimer->start(slowLaneInterval, true);

  addEventSource(m_fastLaneTimer);
  addEventSource(m_paceLaneTimer);
  addEventSource(m_slowLaneTimer);
}

void Application::startWatchdog(void)
{
#ifdef WITH_SYSTEMD
  ulong usec = 0;
  int status;

  status = sd_watchdog_enabled(0, &usec);
  if (status > 0) {
    logInfo() << "Systemd watchdog enabled with timeout seconds: " << USEC2SEC(usec);

    auto timer = std::make_shared<Timer>("Watchdog", [this]() {
      if (sd_notify(0, "WATCHDOG=1") < 0) {
        logWarn() << "Fail to send the heartbeet to systemd";
      } else {
        logDebug() << "Watchdog heartbeat sent";
      }

      return true;
    });

    timer->start((usec / 2), true);
    App()->addEventSource(timer);
  } else {
    if (status == 0) {
      logInfo() << "Systemd watchdog disabled";
    } else {
      logWarn() << "Fail to get the systemd watchdog status";
    }
  }
#else
  logInfo() << "Watchdog build time disabled";
#endif
}

static bool shouldStartTCPServer(const std::shared_ptr<tkm::monitor::Options> opts)
{
  if (opts->getFor(Options::Key::TCPServerStartIfPath) != "none") {
    std::filesystem::path condPath(opts->getFor(Options::Key::TCPServerStartIfPath));
    if (std::filesystem::exists(condPath)) {
      return true;
    }
    return false;
  }

  return true;
}

static bool shouldStartUDSServer(const std::shared_ptr<tkm::monitor::Options> opts)
{
  if (opts->getFor(Options::Key::TCPServerStartIfPath) != "none") {
    std::filesystem::path condPath(opts->getFor(Options::Key::TCPServerStartIfPath));
    if (std::filesystem::exists(condPath)) {
      return true;
    }
    return false;
  }

  return true;
}

} // namespace tkm::monitor
