/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     TCPServer Class
 * @details   Server listening to TCPCollector connections
 *-
 */

#pragma once

#include <TaskMonitor.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "ICollector.h"
#include "Options.h"

#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class TCPServer : public Pollable, public std::enable_shared_from_this<TCPServer>
{
public:
  explicit TCPServer(const std::shared_ptr<Options> options);
  ~TCPServer();

public:
  TCPServer(TCPServer const &) = delete;
  void operator=(TCPServer const &) = delete;

  auto getShared() -> std::shared_ptr<TCPServer> { return shared_from_this(); }
  void bindAndListen();
  void invalidate();
  void enableEvents();

private:
  std::shared_ptr<Options> m_options = nullptr;
  struct sockaddr_in m_addr {
  };
  int m_sockFd = -1;
  bool m_bound = false;
};

} // namespace tkm::monitor
