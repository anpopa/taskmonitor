/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     UDPServer Class
 * @details   Server to handle UDP data sink
 *-
 */

#pragma once

#include <taskmonitor/EnvelopeWriter.h>
#include <taskmonitor/taskmonitor.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include "IUDPSink.h"
#include "Options.h"

#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class UDPServer : public Pollable, public IUDPSink, public std::enable_shared_from_this<UDPServer>
{
public:
  explicit UDPServer(const std::shared_ptr<Options> options);
  ~UDPServer();

public:
  UDPServer(UDPServer const &) = delete;
  void operator=(UDPServer const &) = delete;

  auto getShared() -> std::shared_ptr<UDPServer> { return shared_from_this(); }
  void setEventSource(bool enabled = true);
  void bindAndListen();
  void invalidate();

  bool active(void) { return m_bound; };
  bool send(const tkm::msg::monitor::Data &data);

private:
  std::shared_ptr<Options> m_options = nullptr;
  std::unique_ptr<tkm::EnvelopeWriter> m_writer = nullptr;
  struct sockaddr_in m_addr {
  };
  int m_sockFd = -1;
  bool m_bound = false;
};

} // namespace tkm::monitor
