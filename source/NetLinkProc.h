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

#include <linux/netlink.h>
#include <string>
#include <sys/socket.h>

#include "Options.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/IApplication.h"
#include "../bswinfra/source/Pollable.h"

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

public:
    auto getShared() -> std::shared_ptr<NetLinkProc> { return shared_from_this(); }
    void enableEvents();
    auto startProcMonitoring(void) -> int;
    [[nodiscard]] int getFD() const { return m_sockFd; }

private:
    std::shared_ptr<Options> m_options = nullptr;
    struct sockaddr_nl m_addr = {};
    int m_sockFd = -1;
};

} // namespace tkm::monitor
