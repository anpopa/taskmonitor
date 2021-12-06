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

#include <netinet/in.h>
#include <netlink/netlink.h>
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

class NetLinkStats : public Pollable, public std::enable_shared_from_this<NetLinkStats>
{
public:
    explicit NetLinkStats(std::shared_ptr<Options> &options);
    ~NetLinkStats();

public:
    NetLinkStats(NetLinkStats const &) = delete;
    void operator=(NetLinkStats const &) = delete;

    auto getShared() -> std::shared_ptr<NetLinkStats> { return shared_from_this(); }
    void enableEvents();
    [[nodiscard]] int getFD() const { return m_sockFd; }

    auto requestTaskAcct(int pid) -> int;

private:
    std::shared_ptr<Options> m_options = nullptr;
    struct nl_sock *m_nlSock = nullptr;
    int m_nlFamily = 0;
    int m_sockFd = -1;
};

} // namespace tkm::monitor
