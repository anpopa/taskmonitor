/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     NetLinkStats Class
 * @details   Collect process statistics for each ProcEntry
 *-
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

public:
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
