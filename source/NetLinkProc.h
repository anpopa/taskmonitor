/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     NetLinkProc Class
 * @details   Monitor system process events using netlink interfaces
 *-
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
