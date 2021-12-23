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

#include "Application.h"
#ifdef WITH_WATCHDOG
#include <systemd/sd-daemon.h>
#endif

using std::shared_ptr;
using std::string;

#define USEC2SEC(x) (x / 1000000)

namespace tkm::monitor
{

Application *Application::appInstance = nullptr;

Application::Application(const string &name, const string &description, const string &configFile)
: bswi::app::IApplication(name, description)
{
    if (Application::appInstance != nullptr) {
        throw bswi::except::SingleInstance();
    }
    appInstance = this;

    m_options = std::make_shared<Options>(configFile);

    m_nlStats = std::make_shared<NetLinkStats>(m_options);
    m_nlStats->enableEvents();

    m_nlProc = std::make_shared<NetLinkProc>(m_options);
    m_nlProc->enableEvents();

    m_registry = std::make_shared<Registry>(m_options);
    m_sysProcStat = std::make_shared<SysProcStat>(m_options);

    m_manager = std::make_unique<ActionManager>(m_options, m_nlStats, m_nlProc);
    m_manager->enableEvents();

    startWatchdog();
}

void Application::startWatchdog(void)
{
#ifdef WITH_WATCHDOG
    ulong usec = 0;
    int status;

    status = sd_watchdog_enabled(0, &usec);
    if (status > 0) {
        logInfo() << "Systemd watchdog enabled with timeout %lu seconds" << USEC2SEC(usec);

        auto timer = std::make_shared<Timer>("Watchdog", [this]() {
            if (sd_notify(0, "WATCHDOG=1") < 0) {
                logWarn() << "Fail to send the heartbeet to systemd";
            } else {
                logDebug() << "Watchdog heartbeat sent";
            }

            return true;
        });

        timer->start((usec / 2), true);
        TaskMonitor()->addEventSource(timer);
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

} // namespace tkm::monitor
