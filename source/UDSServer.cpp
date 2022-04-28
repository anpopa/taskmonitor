/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     UDSServer Class
 * @details   Unix domain server
 *-
 */

#include <filesystem>
#include <unistd.h>

#include "Application.h"
#include "Helpers.h"
#include "UDSCollector.h"
#include "UDSServer.h"

namespace fs = std::filesystem;

namespace tkm::monitor
{

UDSServer::UDSServer(const std::shared_ptr<Options> options)
: Pollable("UDSServer")
, m_options(options)
{
  if ((m_sockFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    throw std::runtime_error("Fail to create UDSServer socket");
  }

  lateSetup(
      [this]() {
        int clientFd = accept(m_sockFd, (struct sockaddr *) nullptr, nullptr);

        if (clientFd < 0) {
          logWarn() << "Fail to accept on UDSServer socket";
          return false;
        }

        // The client has 3 seconds to send the descriptor or will be
        // disconnected
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));

        tkm::msg::collector::Descriptor descriptor{};

        if (!readCollectorDescriptor(clientFd, descriptor)) {
          logWarn() << "UDS collector" << clientFd << " read descriptor failed";
          close(clientFd);
          return true; // this is a client issue, process next client
        }

        logInfo() << "New UDSCollector with FD: " << clientFd;
        std::shared_ptr<UDSCollector> collector = std::make_shared<UDSCollector>(clientFd);
        collector->enableEvents();

        return true;
      },
      m_sockFd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // We are ready for events only after start
  setPrepare([]() { return false; });
}

void UDSServer::enableEvents()
{
  App()->addEventSource(getShared());
}

UDSServer::~UDSServer()
{
  static_cast<void>(stop());
}

void UDSServer::start()
{
  fs::path sockPath(m_options->getFor(Options::Key::UDSServerSocketPath));

  m_addr.sun_family = AF_UNIX;
  strncpy(m_addr.sun_path, sockPath.c_str(), sizeof(m_addr.sun_path));

  if (fs::exists(sockPath)) {
    logWarn() << "Runtime directory not clean, removing " << sockPath.string();
    if (!fs::remove(sockPath)) {
      throw std::runtime_error("Fail to remove existing UDSServer socket");
    }
  }

  if (bind(m_sockFd, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_un)) != -1) {
    // We are ready for events only after start
    setPrepare([]() { return true; });
    if (listen(m_sockFd, 10) == -1) {
      logError() << "UDSServer listening failed on " << sockPath.string()
                 << ". Error: " << strerror(errno);
      throw std::runtime_error("UDSServer server listen failed");
    }
    logInfo() << "Control server listening on " << sockPath.string();
  } else {
    logError() << "UDSServer bind failed on " << sockPath.string()
               << ". Error: " << strerror(errno);
    throw std::runtime_error("UDSServer server bind failed");
  }
}

void UDSServer::stop()
{
  if (m_sockFd > 0) {
    ::close(m_sockFd);
    m_sockFd = -1;
  }
}

} // namespace tkm::monitor
