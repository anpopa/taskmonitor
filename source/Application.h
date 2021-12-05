/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#pragma once

#include <atomic>
#include <cstdlib>
#include <string>

#include "ActionManager.h"
#include "Defaults.h"
#include "NetLink.h"
#include "Options.h"

#include "../bswinfra/source/IApplication.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/EventLoop.h"
#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/KeyFile.h"
#include "../bswinfra/source/Logger.h"
#include "../bswinfra/source/PathEvent.h"
#include "../bswinfra/source/Pollable.h"
#include "../bswinfra/source/Timer.h"
#include "../bswinfra/source/UserEvent.h"

using namespace bswi::kf;
using namespace bswi::log;
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
    auto getManager() -> std::shared_ptr<ActionManager> { return m_manager; }
    auto hasConfigFile() -> bool { return m_options->hasConfigFile(); }
    auto getConfigFile() -> std::shared_ptr<bswi::kf::KeyFile>
    {
        return m_options->getConfigFile();
    }

public:
    Application(Application const &) = delete;
    void operator=(Application const &) = delete;

private:
    std::shared_ptr<Options> m_options = nullptr;
    std::shared_ptr<NetLink> m_netlink = nullptr;
    std::shared_ptr<ActionManager> m_manager = nullptr;

private:
    static Application *appInstance;
};

} // namespace tkm::monitor

#define TaskMonitor() tkm::monitor::Application::getInstance()
