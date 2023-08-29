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

#include <sys/stat.h>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include <taskmonitor/taskmonitor.h>

#include "Application.h"
#include "UDSCollector.h"
#include "UDSServer.h"

namespace tkm::monitor
{

UDSServer::UDSServer(const std::shared_ptr<Options> options)
: Pollable("UDSServer")
, m_options(options)
{
  if ((m_sockFd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    throw std::runtime_error("Fail to create UDSServer socket");
  }

  lateSetup(
      [this, options]() {
        int clientFd = accept(m_sockFd, (struct sockaddr *) nullptr, nullptr);

        if (clientFd < 0) {
          if (errno == EWOULDBLOCK || (EWOULDBLOCK != EAGAIN && errno == EAGAIN)) {
            return true;
          }
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

        logInfo() << "New UD    broadcastAddr.sin_port = htons(broadcastPort);         /* Broadcast port */SCollector with FD: " << clientFd << " ID: " << descriptor.id();
        std::shared_ptr<UDSCollector> collector = std::make_shared<UDSCollector>(clientFd);
        collector->getDescriptor().CopyFrom(descriptor);
        collector->setEventSource();

        // Request StateManager to monitor collector for inactivity
        if (options->getFor(Options::Key::UDSMonitorCollectorInactivity) ==
            tkmDefaults.valFor(Defaults::Val::True)) {
          StateManager::Request monitorRequest = {.action = StateManager::Action::MonitorCollector,
                                                  .collector = collector};
          App()->getStateManager()->pushRequest(monitorRequest);
        }

        return true;
      },
      m_sockFd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // We are ready for events only after start
  setPrepare([]() { return false; });
}

void UDSServer::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(getShared());
  } else {
    App()->remEventSource(getShared());
  }
}

UDSServer::~UDSServer()
{
  static_cast<void>(stop());
}

void UDSServer::start()
{
  fs::path sockPath(m_options->getFor(Options::Key::UDSServerSocketPath));

  m_addr.sun_family = AF_UNIX;
  strncpy(m_addr.sun_path, sockPath.c_str(), sizeof(m_addr.sun_path) - 1);

  if (fs::exists(sockPath)) {
    logWarn() << "Runtime directory not clean, removing " << sockPath.string();
    if (!fs::remove(sockPath)) {
      throw std::runtime_error("Fail to remove existing UDSServer socket");
    }
  }

  if (bind(m_sockFd, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_un)) != -1) {

    // Allow non root processes to connect
    ::chmod(sockPath.c_str(), 0666);

    // We are ready for events only after start
    setPrepare([]() { return true; });
    if (listen(m_sockFd, 10) == -1) {
      logError() << "UDSServer listening failed on " << sockPath.string()
                 << ". Error: " << strerror(errno);
      throw std::runtime_error("UDSServer server listen failed");
    }
    logInfo() << "UDSServer listening on " << sockPath.string();
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
