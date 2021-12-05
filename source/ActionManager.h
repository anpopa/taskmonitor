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

#include <map>
#include <string>

#include "NetLink.h"
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
    explicit ActionManager(std::shared_ptr<Options> &options, std::shared_ptr<NetLink> &netlink);

    auto getShared() -> std::shared_ptr<ActionManager> { return shared_from_this(); }
    void enableEvents();

    auto pushRequest(Request &request) -> int;
    auto getNetLink() -> std::shared_ptr<NetLink> { return m_netlink; }

private:
    auto requestHandler(const Request &request) -> bool;

private:
    std::shared_ptr<Options> m_options = nullptr;
    std::shared_ptr<NetLink> m_netlink = nullptr;
    std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
};

} // namespace tkm::monitor
