/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Alin Popa
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

#include <linux/netlink.h>
#include <string>
#include <sys/socket.h>

#include "Options.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/IApplication.h"
#include "../bswinfra/source/Pollable.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::monitor
{

class NetLinkProc : public Pollable, public std::enable_shared_from_this<NetLinkProc>
{
public:
    explicit NetLinkProc(std::shared_ptr<Options> &options);
    ~NetLinkProc();

public:
    NetLinkProc(NetLinkProc const &) = delete;
    void operator=(NetLinkProc const &) = delete;

    auto getShared() -> std::shared_ptr<NetLinkProc> { return shared_from_this(); }
    void enableEvents();
    [[nodiscard]] int getFD() const { return m_sockFd; }

    auto startProcMonitoring(void) -> int;

private:
    std::shared_ptr<Options> m_options = nullptr;
    struct sockaddr_nl m_addr = {};
    int m_sockFd = -1;
};

} // namespace tkm::monitor
