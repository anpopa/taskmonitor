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

#include <map>
#include <string>

#include "NetLinkProc.h"
#include "NetLinkStats.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/Logger.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ActionManager : public std::enable_shared_from_this<ActionManager>
{
public:
    enum class Action { RegisterEvents };

    typedef struct Request {
        Action action;
        std::map<std::string, std::string> args;
    } Request;

public:
    explicit ActionManager(std::shared_ptr<Options> &options,
                           std::shared_ptr<NetLinkStats> &nlStats,
                           std::shared_ptr<NetLinkProc> &nlProc);

    auto getShared() -> std::shared_ptr<ActionManager> { return shared_from_this(); }
    void enableEvents();

    auto pushRequest(Request &request) -> int;
    auto getNetLinkStats() -> std::shared_ptr<NetLinkStats> { return m_nlStats; }
    auto getNetLinkProc() -> std::shared_ptr<NetLinkProc> { return m_nlProc; }

private:
    auto requestHandler(const Request &request) -> bool;

private:
    std::shared_ptr<Options> m_options = nullptr;
    std::shared_ptr<NetLinkStats> m_nlStats = nullptr;
    std::shared_ptr<NetLinkProc> m_nlProc = nullptr;
    std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
};

} // namespace tkm::monitor
