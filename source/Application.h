/*
 * SPDX license identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2021 Alin Popa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * \author Alin Popa <alin.popa@fxdata.ro>
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

private:
    static Application *appInstance;
};

} // namespace tkm::monitor

#define TaskMonitor() tkm::monitor::Application::getInstance()
