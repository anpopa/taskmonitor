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

#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "Application.h"

#define USEC2SEC(x) (x / 1000000)

namespace tkm::monitor
{

Application *Application::appInstance = nullptr;

static bool isProfMode(const std::shared_ptr<tkm::monitor::Options> opts);

Application::Application(const std::string &name,
                         const std::string &description,
                         const std::string &configFile)
: bswi::app::IApplication(name, description)
{
  if (Application::appInstance != nullptr) {
    throw bswi::except::SingleInstance();
  }
  appInstance = this;

  m_options = std::make_shared<Options>(configFile);
  bool profModeEnabled = isProfMode(m_options);

  // Set update lanes intervals based on runtime mode
  if (profModeEnabled) {
    logInfo() << "Profiling mode enabled";
    m_fastLaneInterval = std::stoul(m_options->getFor(Options::Key::ProfModeFastLaneInt));
    m_paceLaneInterval = std::stoul(m_options->getFor(Options::Key::ProfModePaceLaneInt));
    m_slowLaneInterval = std::stoul(m_options->getFor(Options::Key::ProfModeSlowLaneInt));
  } else {
    m_fastLaneInterval = std::stoul(m_options->getFor(Options::Key::ProdModeFastLaneInt));
    m_paceLaneInterval = std::stoul(m_options->getFor(Options::Key::ProdModePaceLaneInt));
    m_slowLaneInterval = std::stoul(m_options->getFor(Options::Key::ProdModeSlowLaneInt));
  }

  logDebug() << "Update lanes interval fast=" << m_fastLaneInterval
             << " pace=" << m_paceLaneInterval << " slow=" << m_slowLaneInterval;

  if (m_options->getFor(Options::Key::EnableTCPServer) == tkmDefaults.valFor(Defaults::Val::True)) {
    if (profModeEnabled) {
      m_netServer = std::make_shared<TCPServer>(m_options);
      try {
        m_netServer->bindAndListen();
        m_netServer->setEventSource();
      } catch (std::exception &e) {
        logError() << "Fail to start TCP server. Exception: " << e.what();
      }
    }
  }

  if (m_options->getFor(Options::Key::EnableUDSServer) == tkmDefaults.valFor(Defaults::Val::True)) {
    m_udsServer = std::make_shared<UDSServer>(m_options);
    try {
      m_udsServer->start();
      m_udsServer->setEventSource();
    } catch (std::exception &e) {
      logError() << "Fail to start UDS server. Exception: " << e.what();
    }
  }

  if (m_options->getFor(Options::Key::EnableUDPServer) == tkmDefaults.valFor(Defaults::Val::True)) {
    m_udpServer = std::make_shared<UDPServer>(m_options);
    try {
      m_udpServer->bindAndListen();
      m_udpServer->setEventSource();
    } catch (std::exception &e) {
      logError() << "Fail to start UDP server. Exception: " << e.what();
    }
  }

#ifdef WITH_PROC_EVENT
  if (m_options->getFor(Options::Key::EnableProcEvent) == tkmDefaults.valFor(Defaults::Val::True)) {
    m_procEvent = std::make_shared<ProcEvent>(m_options);
    m_procEvent->setEventSource();
  }
#endif

#ifdef WITH_PROC_ACCT
  if (m_options->getFor(Options::Key::EnableProcAcct) == tkmDefaults.valFor(Defaults::Val::True)) {
    if (profModeEnabled) {
      m_procAcct = std::make_shared<ProcAcct>(m_options);
      m_procAcct->setEventSource();
    }
  }
#endif

#ifdef WITH_STARTUP_DATA
  if (m_options->getFor(Options::Key::EnableStartupData) ==
      tkmDefaults.valFor(Defaults::Val::True)) {
    if (profModeEnabled) {
      m_startupData = std::make_shared<StartupData>(m_options);
      m_startupData->setEventSource();
    }
  }
#endif

  // Create state manager
  m_stateManager = std::make_shared<StateManager>(m_options);
  m_stateManager->setEventSource();

  // Create and initialize data sources
  m_procRegistry = std::make_shared<ProcRegistry>(m_options);
  m_procRegistry->setUpdateLane(IDataSource::UpdateLane::Any);
  m_procRegistry->setUpdateInterval(m_paceLaneInterval);
  m_procRegistry->setEventSource();
  m_dataSources.append(m_procRegistry);

  if (fs::exists("/proc/stat")) {
    m_sysProcStat = std::make_shared<SysProcStat>(m_options);
    m_sysProcStat->setUpdateLane(IDataSource::UpdateLane::Fast);
    m_sysProcStat->setUpdateInterval(m_fastLaneInterval);
    m_sysProcStat->setEventSource();
    m_dataSources.append(m_sysProcStat);
  }

  if (fs::exists("/proc/meminfo")) {
    m_sysProcMemInfo = std::make_shared<SysProcMemInfo>(m_options);
    m_sysProcMemInfo->setUpdateLane(IDataSource::UpdateLane::Fast);
    m_sysProcMemInfo->setUpdateInterval(m_fastLaneInterval);
    m_sysProcMemInfo->setEventSource();
    m_dataSources.append(m_sysProcMemInfo);
  }

  if (fs::exists("/proc/pressure")) {
    m_sysProcPressure = std::make_shared<SysProcPressure>(m_options);
    m_sysProcPressure->setUpdateLane(IDataSource::UpdateLane::Pace);
    m_sysProcPressure->setUpdateInterval(m_paceLaneInterval);
    m_sysProcPressure->setEventSource();
    m_dataSources.append(m_sysProcPressure);
  }

  if (fs::exists("/proc/diskstats")) {
    m_sysProcDiskStats = std::make_shared<SysProcDiskStats>(m_options);
    m_sysProcDiskStats->setUpdateLane(IDataSource::UpdateLane::Pace);
    m_sysProcDiskStats->setUpdateInterval(m_paceLaneInterval);
    m_sysProcDiskStats->setEventSource();
    m_dataSources.append(m_sysProcDiskStats);
  }

  if (fs::exists("/proc/buddyinfo")) {
    m_sysProcBuddyInfo = std::make_shared<SysProcBuddyInfo>(m_options);
    m_sysProcBuddyInfo->setUpdateLane(IDataSource::UpdateLane::Slow);
    m_sysProcBuddyInfo->setUpdateInterval(m_slowLaneInterval);
    m_sysProcBuddyInfo->setEventSource();
    m_dataSources.append(m_sysProcBuddyInfo);
  }

  if (fs::exists("/proc/net/wireless")) {
    m_sysProcWireless = std::make_shared<SysProcWireless>(m_options);
    m_sysProcWireless->setUpdateLane(IDataSource::UpdateLane::Slow);
    m_sysProcWireless->setUpdateInterval(m_slowLaneInterval);
    m_sysProcWireless->setEventSource();
    m_dataSources.append(m_sysProcWireless);
  }

#ifdef WITH_VM_STAT
  if ((m_options->getFor(Options::Key::EnableSysProcVMStat) ==
      tkmDefaults.valFor(Defaults::Val::True)) && (fs::exists("/proc/vmstat"))) {
    m_sysProcVMStat = std::make_shared<SysProcVMStat>(m_options);
    m_sysProcVMStat->setUpdateLane(IDataSource::UpdateLane::Slow);
    m_sysProcVMStat->setUpdateInterval(m_slowLaneInterval);
    m_sysProcVMStat->setEventSource();
    m_dataSources.append(m_sysProcVMStat);
  }
#endif

  // Commit our final data source list
  m_dataSources.commit();

  // Create and start lanes timers
  enableUpdateLanes();

  // Enable watchdog timer
  startWatchdog();

  // After init we can lower or priority if configured in production mode
  if (!profModeEnabled) {
    if (m_options->getFor(Options::Key::SelfLowerPriority) ==
        tkmDefaults.valFor(Defaults::Val::True)) {
      if (nice(19) == -1) {
        logWarn() << "Failed to set nice value. Error: " << strerror(errno);
      }
    }
  }
}

void Application::enableUpdateLanes(void)
{
  m_fastLaneTimer = std::make_shared<Timer>("FastLaneTimer", [this]() {
    m_dataSources.foreach ([this](const std::shared_ptr<IDataSource> &entry) {
      if (entry->getUpdateLane() == IDataSource::UpdateLane::Fast) {
        entry->update();
        entry->sendTo(*m_udpServer);
      } else if (entry->getUpdateLane() == IDataSource::UpdateLane::Any) {
        entry->update(IDataSource::UpdateLane::Fast);
      }
    });
    return true;
  });

  m_paceLaneTimer = std::make_shared<Timer>("PaceLaneTimer", [this]() {
    m_dataSources.foreach ([](const std::shared_ptr<IDataSource> &entry) {
      if (entry->getUpdateLane() == IDataSource::UpdateLane::Pace) {
        entry->update();
      } else if (entry->getUpdateLane() == IDataSource::UpdateLane::Any) {
        entry->update(IDataSource::UpdateLane::Pace);
      }
    });
    return true;
  });

  m_slowLaneTimer = std::make_shared<Timer>("SlowLaneTimer", [this]() {
    m_dataSources.foreach ([](const std::shared_ptr<IDataSource> &entry) {
      if (entry->getUpdateLane() == IDataSource::UpdateLane::Slow) {
        entry->update();
      } else if (entry->getUpdateLane() == IDataSource::UpdateLane::Any) {
        entry->update(IDataSource::UpdateLane::Slow);
      }
    });
    return true;
  });

  m_fastLaneTimer->start(m_fastLaneInterval, true);
  m_paceLaneTimer->start(m_paceLaneInterval, true);
  m_slowLaneTimer->start(m_slowLaneInterval, true);
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

    auto timer = std::make_shared<Timer>("Watchdog", []() {
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

static bool isProfMode(const std::shared_ptr<tkm::monitor::Options> opts)
{
  auto profCond = opts->getFor(Options::Key::ProfModeIfPath);
  if (profCond != tkmDefaults.valFor(Defaults::Val::None)) {
    if (fs::exists(profCond)) {
      return true;
    }
  }
  return false;
}

} // namespace tkm::monitor
