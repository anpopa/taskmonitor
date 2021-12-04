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

#include <filesystem>
#include <unistd.h>

#include "ActionManager.h"
#include "Application.h"
#include "Defaults.h"

using std::shared_ptr;
using std::string;
namespace fs = std::filesystem;

namespace tkm::monitor
{

static auto doActionConnect(ActionManager *manager, const ActionManager::Request &request) -> bool;

ActionManager::ActionManager(shared_ptr<Options> &options, shared_ptr<NetLink> &netlink)
: m_options(options)
, m_netlink(netlink)
{
    m_queue = std::make_shared<AsyncQueue<Request>>(
        "ActionManagerQueue", [this](const Request &request) { return requestHandler(request); });

    TaskMonitor()->addEventSource(m_queue);
}

auto ActionManager::pushRequest(Request &request) -> int
{
    return m_queue->push(request);
}

auto ActionManager::requestHandler(const Request &request) -> bool
{
    switch (request.action) {
    case ActionManager::Action::Connect:
        return doActionConnect(this, request);
    default:
        break;
    }

    logError() << "Unknown action request";
    return false;
}

static auto doActionConnect(ActionManager *manager, const ActionManager::Request &) -> bool
{
    if (manager->getNetLink()->connect() < 0) {
        return false;
    }

    return true;
}

} // namespace tkm::monitor
