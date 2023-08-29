/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     TCPServer Class
 * @details   Server listening to UDP TCPCollector connections
 *-
 */

#include <errno.h>
#include <netdb.h>
#include <taskmonitor/taskmonitor.h>
#include <unistd.h>

#include "Application.h"
#include "StateManager.h"
#include "TCPCollector.h"
#include "TCPServer.h"

namespace tkm::monitor
{

TCPServer::TCPServer(const std::shared_ptr<Options> options)
: Pollable("TCPServer")
, m_options(options)
{
  if ((m_sockFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    throw std::runtime_error("Fail to create TCPServer socket");
  }

  int enable = 1;
  setsockopt(m_sockFd,
             SOL_SOCKET,
             (SO_REUSEPORT | SO_REUSEADDR),
             (char *) &enable,
             sizeof(enable));

  lateSetup(
      [this]() {
        int collectorFd = accept(m_sockFd, (struct sockaddr *) nullptr, nullptr);

        if (collectorFd < 0) {
          if (errno == EWOULDBLOCK || (EWOULDBLOCK != EAGAIN && errno == EAGAIN)) {
            return true;
          }
          logWarn() << "Fail to accept on TCPServer socket";
          return false;
        }

        // The collector has 3 seconds to send the descriptor or will be disconnected
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        setsockopt(collectorFd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));

        tkm::msg::collector::Descriptor descriptor{};
        if (!readCollectorDescriptor(collectorFd, descriptor)) {
          logWarn() << "Collector " << collectorFd << " read descriptor failed";
          close(collectorFd);
          return true; // this is a collector issue, process next collector
        }

        logInfo() << "New Collector with FD: " << collectorFd;
        std::shared_ptr<TCPCollector> collector = std::make_shared<TCPCollector>(collectorFd);
        collector->getDescriptor().CopyFrom(descriptor);
        collector->setEventSource();

        // Request StateManager to monitor collector for inactivity
        StateManager::Request monitorRequest = {.action = StateManager::Action::MonitorCollector,
                                                .collector = collector};
        App()->getStateManager()->pushRequest(monitorRequest);

        return true;
      },
      m_sockFd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // We are ready for events only after start
  setPrepare([]() { return false; });
}

void TCPServer::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(getShared());
  } else {
    App()->remEventSource(getShared());
  }
}

TCPServer::~TCPServer()
{
  static_cast<void>(invalidate());
}

void TCPServer::bindAndListen()
{
  int port = -1;

  if (m_bound) {
    logWarn() << "TCPServer already listening";
    return;
  }

  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;

  if (m_options->getFor(Options::Key::TCPServerAddress) != "any") {
    std::string serverAddress = m_options->getFor(Options::Key::TCPServerAddress);
    struct hostent *server = gethostbyname(serverAddress.c_str());
    memcpy(&m_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
  }

  try {
    port = std::stoi(m_options->getFor(Options::Key::TCPServerPort));
  } catch (const std::exception &e) {
    port = std::stoi(tkmDefaults.getFor(Defaults::Default::TCPServerPort));
    logWarn() << "Cannot convert port number from config: " << e.what();
  }
  m_addr.sin_port = htons(static_cast<uint16_t>(port));

  if (bind(m_sockFd, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_in)) != -1) {
    setPrepare([]() { return true; });
    if (listen(m_sockFd, 10) == -1) {
      logError() << "TCPServer listening failed on port: " << port
                 << ". Error: " << strerror(errno);
      throw std::runtime_error("TCPServer listen failed");
    }
    logInfo() << "TCPServer listening on port: " << port;
  } else {
    logError() << "TCPServer bind failed on port: " << port << ". Error: " << strerror(errno);
    throw std::runtime_error("TCPServer bind failed");
  }

  m_bound = true;
}

void TCPServer::invalidate()
{
  if (m_sockFd > 0) {
    ::close(m_sockFd);
    m_sockFd = -1;
  }
}

} // namespace tkm::monitor
