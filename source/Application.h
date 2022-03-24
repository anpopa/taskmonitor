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

#include "ActionManager.h"
#include "Defaults.h"
#include "NetLinkProc.h"
#include "NetLinkStats.h"
#include "Options.h"
#include "Registry.h"
#include "SysProcPressure.h"
#include "SysProcStat.h"

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
    auto getOptions() -> std::shared_ptr<Options> { return m_options; }
    auto getManager() -> std::shared_ptr<ActionManager> { return m_manager; }
    auto getRegistry() -> std::shared_ptr<Registry> { return m_registry; }
    auto getSysProcStat() -> std::shared_ptr<SysProcStat> { return m_sysProcStat; }
    auto getSysProcPressure() -> std::shared_ptr<SysProcPressure> { return m_sysProcPressure; }
    auto hasConfigFile() -> bool { return m_options->hasConfigFile(); }
    auto getConfigFile() -> std::shared_ptr<bswi::kf::KeyFile>
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
    std::shared_ptr<NetLinkStats> m_nlStats = nullptr;
    std::shared_ptr<NetLinkProc> m_nlProc = nullptr;
    std::shared_ptr<ActionManager> m_manager = nullptr;
    std::shared_ptr<Registry> m_registry = nullptr;
    std::shared_ptr<SysProcStat> m_sysProcStat = nullptr;
    std::shared_ptr<SysProcPressure> m_sysProcPressure = nullptr;

private:
    static Application *appInstance;
};

} // namespace tkm::monitor

#define TaskMonitor() tkm::monitor::Application::getInstance()
