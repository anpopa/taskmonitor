/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     TCPServer Class
 * @details   Server listening to TCPClient connections
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

class TCPServer : public Pollable, public std::enable_shared_from_this<TCPServer>
{
public:
  TCPServer();
  ~TCPServer();

public:
  TCPServer(TCPServer const &) = delete;
  void operator=(TCPServer const &) = delete;

  void bindAndListen();
  void invalidate();
  void enableEvents();
  auto getShared() -> std::shared_ptr<TCPServer> { return shared_from_this(); }
  void sendData(const tkm::msg::server::Data &data);
  void notifyClientTerminated(int id);
  bool hasClients(void) { return (m_clients.getSize() > 0); }

private:
  struct sockaddr_in m_addr {
  };
  bswi::util::SafeList<std::shared_ptr<IClient>> m_clients{"ClientList"};
  int m_sockFd = -1;
  bool m_bound = false;
};

} // namespace tkm::monitor
