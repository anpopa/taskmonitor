/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     NetServer Class
 * @details   Server listening to UDP NetClient connections
 *-
 */

#pragma once

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

#include "IClient.h"
#include "Options.h"

#include "../bswinfra/source/IApplication.h"
#include "../bswinfra/source/Pollable.h"
#include "../bswinfra/source/SafeList.h"

#include "Server.pb.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::monitor
{

class NetServer : public Pollable, public std::enable_shared_from_this<NetServer>
{
public:
    NetServer();
    ~NetServer();

public:
    NetServer(NetServer const &) = delete;
    void operator=(NetServer const &) = delete;

    void bindAndListen();
    void invalidate();
    void enableEvents();
    auto getShared() -> std::shared_ptr<NetServer> { return shared_from_this(); }
    void sendData(const tkm::msg::server::Data &data);
    void notifyClientTerminated(int id);

private:
    struct sockaddr_in m_addr {
    };
    bswi::util::SafeList<std::shared_ptr<IClient>> m_clients {"ClientList"};
    int m_sockFd = -1;
    bool m_bound = false;
};

} // namespace tkm::monitor
