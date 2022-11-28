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

#include "IDataSource.h"
#include "Options.h"
#ifdef WITH_PROC_ACCT
#include "ProcAcct.h"
#endif
#ifdef WITH_PROC_EVENT
#include "ProcEvent.h"
#endif
#ifdef WITH_STARTUP_DATA
#include "StartupData.h"
#endif
#include "ProcEntry.h"
#include "ProcRegistry.h"
#include "SysProcBuddyInfo.h"
#include "SysProcDiskStats.h"
#include "SysProcMemInfo.h"
#include "SysProcPressure.h"
#include "SysProcStat.h"
#include "SysProcWireless.h"
#include "TCPServer.h"
#include "UDSServer.h"

#include "../../bswinfra/source/IApplication.h"
#include "../../bswinfra/source/SafeList.h"
#include "../../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{

class Application final : public bswi::app::IApplication
{
public:
  explicit Application(const std::string &name,
                       const std::string &description,
                       const std::string &configFile);

  ~Application()
  {
    this->stop();
    appInstance = nullptr;
  }

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
      m_mainEventLoop->stop();
    }
  }
  auto getOptions(void) -> const std::shared_ptr<Options> { return m_options; }
  auto getTCPServer(void) -> const std::shared_ptr<TCPServer> { return m_netServer; }
  auto getUDSServer(void) -> const std::shared_ptr<UDSServer> { return m_udsServer; }
  auto getProcRegistry(void) -> const std::shared_ptr<ProcRegistry> { return m_procRegistry; }
#ifdef WITH_PROC_ACCT
  auto getProcAcct(void) -> const std::shared_ptr<ProcAcct>
  {
    return m_procAcct;
  }
#endif
#ifdef WITH_PROC_EVENT
  auto getProcEvent(void) -> const std::shared_ptr<ProcEvent>
  {
    return m_procEvent;
  }
#endif
#ifdef WITH_STARTUP_DATA
  auto getStartupData(void) -> const std::shared_ptr<StartupData>
  {
    return m_startupData;
  }
#endif
  auto getSysProcStat(void) -> const std::shared_ptr<SysProcStat>
  {
    return m_sysProcStat;
  }
  auto getSysProcMemInfo(void) -> const std::shared_ptr<SysProcMemInfo>
  {
    return m_sysProcMemInfo;
  }
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
  auto getProcAcctCollectorCounter(void) -> unsigned short
  {
    return m_procAcctCollectorCounter;
  }

public:
  Application(Application const &) = delete;
  void operator=(Application const &) = delete;

public:
  std::shared_ptr<Options> m_options = nullptr;
  std::shared_ptr<TCPServer> m_netServer = nullptr;
  std::shared_ptr<UDSServer> m_udsServer = nullptr;
#ifdef WITH_PROC_ACCT
  std::shared_ptr<ProcAcct> m_procAcct = nullptr;
#endif
#ifdef WITH_PROC_EVENT
  std::shared_ptr<ProcEvent> m_procEvent = nullptr;
#endif
#ifdef WITH_STARTUP_DATA
  std::shared_ptr<StartupData> m_startupData = nullptr;
#endif
  std::shared_ptr<ProcRegistry> m_procRegistry = nullptr;
  std::shared_ptr<SysProcStat> m_sysProcStat = nullptr;
  std::shared_ptr<SysProcMemInfo> m_sysProcMemInfo = nullptr;
  std::shared_ptr<SysProcDiskStats> m_sysProcDiskStats = nullptr;
  std::shared_ptr<SysProcPressure> m_sysProcPressure = nullptr;
  std::shared_ptr<SysProcBuddyInfo> m_sysProcBuddyInfo = nullptr;
  std::shared_ptr<SysProcWireless> m_sysProcWireless = nullptr;
  std::atomic<unsigned short> m_procAcctCollectorCounter = 0;

private:
  static Application *appInstance;
};

} // namespace tkm::monitor

#define App() tkm::monitor::Application::getInstance()
