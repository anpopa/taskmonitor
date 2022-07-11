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

#pragma once

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "Defaults.h"
#include "Dispatcher.h"
#include "IDataSource.h"
#include "Options.h"
#include "ProcAcct.h"
#include "ProcEvent.h"
#include "ProcRegistry.h"
#include "SysProcBuddyInfo.h"
#include "SysProcDiskStats.h"
#include "SysProcMemInfo.h"
#include "SysProcPressure.h"
#include "SysProcStat.h"
#include "SysProcWireless.h"
#include "TCPServer.h"
#include "UDSServer.h"

#include "../bswinfra/source/IApplication.h"
#include "../bswinfra/source/SafeList.h"
#include "../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{

class Application final : public bswi::app::IApplication
{
public:
  explicit Application(const std::string &name,
                       const std::string &description,
                       const std::string &configFile);

  static Application *getInstance()
  {
    return appInstance == nullptr
               ? appInstance = new Application("TKM",
                                               "TaskMonitor Application",
                                               tkmDefaults.getFor(Defaults::Default::ConfPath))
               : appInstance;
  }

  void stop() final
  {
    if (m_running) {
      m_mainEventLoop.reset();
    }
  }
  auto getOptions(void) -> const std::shared_ptr<Options> { return m_options; }
  auto getTCPServer(void) -> const std::shared_ptr<TCPServer> { return m_netServer; }
  auto getUDSServer(void) -> const std::shared_ptr<UDSServer> { return m_udsServer; }
  auto getDispatcher(void) -> const std::shared_ptr<Dispatcher> { return m_dispatcher; }
  auto getProcRegistry(void) -> const std::shared_ptr<ProcRegistry> { return m_procRegistry; }
  auto getProcAcct(void) -> const std::shared_ptr<ProcAcct> { return m_procAcct; }
  auto getProcEvent(void) -> const std::shared_ptr<ProcEvent> { return m_procEvent; }
  auto getSysProcStat(void) -> const std::shared_ptr<SysProcStat> { return m_sysProcStat; }
  auto getSysProcMemInfo(void) -> const std::shared_ptr<SysProcMemInfo> { return m_sysProcMemInfo; }
  auto getSysProcWireless(void) -> const std::shared_ptr<SysProcWireless>
  {
    return m_sysProcWireless;
  }
  auto getSysProcDiskStats(void) -> const std::shared_ptr<SysProcDiskStats>
  {
    return m_sysProcDiskStats;
  }
  auto getSysProcPressure(void) -> const std::shared_ptr<SysProcPressure>
  {
    return m_sysProcPressure;
  }
  auto getSysProcBuddyInfo(void) -> const std::shared_ptr<SysProcBuddyInfo>
  {
    return m_sysProcBuddyInfo;
  }
  bool hasConfigFile(void) { return m_options->hasConfigFile(); }
  auto getConfigFile(void) -> const std::shared_ptr<bswi::kf::KeyFile>
  {
    return m_options->getConfigFile();
  }

  void incProcAcctCollectorCounter(void) { m_procAcctCollectorCounter++; }
  void decProcAcctCollectorCounter(void) { m_procAcctCollectorCounter--; }
  auto getProcAcctCollectorCounter(void) -> unsigned short { return m_procAcctCollectorCounter; }

  auto getFastLaneInterval(void) -> uint64_t { return m_fastLaneInterval; }
  auto getPaceLaneInterval(void) -> uint64_t { return m_paceLaneInterval; }
  auto getSlowLaneInterval(void) -> uint64_t { return m_slowLaneInterval; }

public:
  Application(Application const &) = delete;
  void operator=(Application const &) = delete;

private:
  void startWatchdog(void);
  void enableUpdateLanes(void);

private:
  std::shared_ptr<Options> m_options = nullptr;
  std::shared_ptr<TCPServer> m_netServer = nullptr;
  std::shared_ptr<UDSServer> m_udsServer = nullptr;
  std::shared_ptr<ProcAcct> m_procAcct = nullptr;
  std::shared_ptr<ProcEvent> m_procEvent = nullptr;
  std::shared_ptr<Dispatcher> m_dispatcher = nullptr;
  std::shared_ptr<ProcRegistry> m_procRegistry = nullptr;
  std::shared_ptr<SysProcStat> m_sysProcStat = nullptr;
  std::shared_ptr<SysProcMemInfo> m_sysProcMemInfo = nullptr;
  std::shared_ptr<SysProcDiskStats> m_sysProcDiskStats = nullptr;
  std::shared_ptr<SysProcPressure> m_sysProcPressure = nullptr;
  std::shared_ptr<SysProcBuddyInfo> m_sysProcBuddyInfo = nullptr;
  std::shared_ptr<SysProcWireless> m_sysProcWireless = nullptr;
  std::atomic<unsigned short> m_procAcctCollectorCounter = 0;

private:
  bswi::util::SafeList<std::shared_ptr<IDataSource>> m_dataSources{"DataSourceList"};
  std::shared_ptr<Timer> m_fastLaneTimer = nullptr;
  std::shared_ptr<Timer> m_paceLaneTimer = nullptr;
  std::shared_ptr<Timer> m_slowLaneTimer = nullptr;
  uint64_t m_fastLaneInterval = 10000000;
  uint64_t m_paceLaneInterval = 30000000;
  uint64_t m_slowLaneInterval = 60000000;

private:
  static Application *appInstance;
};

} // namespace tkm::monitor

#define App() tkm::monitor::Application::getInstance()
