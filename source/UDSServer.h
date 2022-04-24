/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     UDSServer Class
 * @details   Unix domain server server
 *-
 */

#pragma once

#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "Options.h"

#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class UDSServer : public Pollable, public std::enable_shared_from_this<UDSServer>
{
public:
  explicit UDSServer(std::shared_ptr<Options> &options);
  ~UDSServer();

  auto getShared(void) -> std::shared_ptr<UDSServer> { return shared_from_this(); }
  void enableEvents(void);
  void start(void);
  void stop(void);

public:
  UDSServer(UDSServer const &) = delete;
  void operator=(UDSServer const &) = delete;

private:
  std::shared_ptr<Options> m_options = nullptr;
  struct sockaddr_un m_addr {
  };
  int m_sockFd = -1;
};

} // namespace tkm::monitor
