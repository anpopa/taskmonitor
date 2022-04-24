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
#include <cstdlib>
#include <string>

#include "Defaults.h"
#include "Dispatcher.h"
#include "Options.h"
#include "ProcAcct.h"
#include "ProcEvent.h"
#include "Registry.h"
#include "SysProcMeminfo.h"
#include "SysProcPressure.h"
#include "SysProcStat.h"
#include "TCPServer.h"

#include "../bswinfra/source/IApplication.h"

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
  auto getOptions() -> std::shared_ptr<Options> & { return m_options; }
  auto getTCPServer() -> std::shared_ptr<TCPServer> & { return m_netServer; }
  auto getDispatcher() -> std::shared_ptr<Dispatcher> & { return m_dispatcher; }
  auto getRegistry() -> std::shared_ptr<Registry> & { return m_registry; }
  auto getProcAcct() -> std::shared_ptr<ProcAcct> & { return m_procAcct; }
  auto getProcEvent() -> std::shared_ptr<ProcEvent> & { return m_procEvent; }
  auto getSysProcStat() -> std::shared_ptr<SysProcStat> & { return m_sysProcStat; }
  auto getSysProcMeminfo() -> std::shared_ptr<SysProcMeminfo> & { return m_sysProcMeminfo; }
  auto getSysProcPressure() -> std::shared_ptr<SysProcPressure> & { return m_sysProcPressure; }
  bool hasConfigFile() { return m_options->hasConfigFile(); }
  auto getConfigFile() -> std::shared_ptr<bswi::kf::KeyFile> &
  {
    return m_options->getConfigFile();
  }

public:
  Application(Application const &) = delete;
  void operator=(Application const &) = delete;

private:
  void startWatchdog(void);

private:
  std::shared_ptr<Options> m_options = nullptr;
  std::shared_ptr<TCPServer> m_netServer = nullptr;
  std::shared_ptr<ProcAcct> m_procAcct = nullptr;
  std::shared_ptr<ProcEvent> m_procEvent = nullptr;
  std::shared_ptr<Dispatcher> m_dispatcher = nullptr;
  std::shared_ptr<Registry> m_registry = nullptr;
  std::shared_ptr<SysProcStat> m_sysProcStat = nullptr;
  std::shared_ptr<SysProcMeminfo> m_sysProcMeminfo = nullptr;
  std::shared_ptr<SysProcPressure> m_sysProcPressure = nullptr;

private:
  static Application *appInstance;
};

} // namespace tkm::monitor

#define App() tkm::monitor::Application::getInstance()
