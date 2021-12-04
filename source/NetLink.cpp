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

#include <csignal>
#include <filesystem>
#include <netdb.h>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "NetLink.h"

namespace fs = std::filesystem;
using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

NetLink::NetLink(std::shared_ptr<Options> &options)
: Pollable("NetLink")
, m_options(options)
{
    if ((m_sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("Fail to create netlink socket");
    }

    lateSetup([this]() { return true; },
              m_sockFd,
              bswi::event::IPollable::Events::Level,
              bswi::event::IEventSource::Priority::Normal);

    // We are ready for events only after connect
    setPrepare([]() { return false; });
    // If the event is removed we stop the main application
    setFinalize([]() {
        logInfo() << "Server closed connection. Terminate";
        ::raise(SIGTERM);
    });
}

NetLink::~NetLink()
{
    if (m_sockFd > 0) {
        ::close(m_sockFd);
    }
}

auto NetLink::connect() -> int
{
    logInfo() << "Connected to server";

    // We are ready for events
    setPrepare([]() { return true; });

    return 0;
}

} // namespace tkm::monitor
