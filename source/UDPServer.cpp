/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     UDPServer Class
 * @details   Server listening to UDP connections
 *-
 */

#include <errno.h>
#include <memory>
#include <taskmonitor/EnvelopeWriter.h>
#include <unistd.h>

#include "Application.h"
#include "Logger.h"
#include "UDPServer.h"

namespace tkm::monitor
{

UDPServer::UDPServer(const std::shared_ptr<Options> options)
: Pollable("UDPServer")
, m_options(options)
{
  if ((m_sockFd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0) {
    throw std::runtime_error("Fail to create UDPServer socket");
  }

  int enable = 1;
  setsockopt(m_sockFd,
             SOL_SOCKET,
             (SO_REUSEPORT | SO_REUSEADDR | SO_BROADCAST),
             (char *) &enable,
             sizeof(enable));

  m_writer = std::make_unique<tkm::EnvelopeWriter>(m_sockFd);

  lateSetup(
      [this]() {
        // TODO: Handle UDP data
        logInfo() << "UDP new client";
        return true;
      },
      m_sockFd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // We are ready for events only after start
  setPrepare([]() { return false; });
}

void UDPServer::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(getShared());
  } else {
    App()->remEventSource(getShared());
  }
}

UDPServer::~UDPServer()
{
  static_cast<void>(invalidate());
}

void UDPServer::bindAndListen()
{
  int port = -1;

  if (m_bound) {
    logWarn() << "UDPServer already listening";
    return;
  }

  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;

  if (m_options->getFor(Options::Key::UDPServerAddress) != "any") {
    std::string serverAddress = m_options->getFor(Options::Key::UDPServerAddress);
    struct hostent *server = gethostbyname(serverAddress.c_str());
    memcpy(&m_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
  }

  try {
    port = std::stoi(m_options->getFor(Options::Key::UDPServerPort));
  } catch (const std::exception &e) {
    port = std::stoi(tkmDefaults.getFor(Defaults::Default::UDPServerPort));
    logWarn() << "Cannot convert port number from config: " << e.what();
  }
  m_addr.sin_port = htons(static_cast<uint16_t>(port));

  if (bind(m_sockFd, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_in)) != -1) {
    setPrepare([]() { return true; });
    logInfo() << "UDPServer listening on port: " << port;
  } else {
    logError() << "UDPServer bind failed on port: " << port << ". Error: " << strerror(errno);
    throw std::runtime_error("UDPServer bind failed");
  }

  m_bound = true;
}

void UDPServer::invalidate()
{
  if (m_sockFd > 0) {
    ::close(m_sockFd);
    m_sockFd = -1;
    m_bound = false;
    m_writer.reset();
  }
}

bool UDPServer::send(const tkm::msg::monitor::Data &data)
{
  tkm::msg::Envelope envelope;
  tkm::msg::monitor::Message message;

  message.set_type(tkm::msg::monitor::Message_Type_Data);
  message.mutable_payload()->PackFrom(data);
  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Monitor);

  if (m_writer->send(envelope) == IAsyncEnvelope::Status::Ok) {
    return m_writer->flush();
  }

  return false;
}

} // namespace tkm::monitor
