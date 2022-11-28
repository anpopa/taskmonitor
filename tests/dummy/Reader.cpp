/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Reader Class
 * @details   Dummy reader implementation
 *-
 */

#include "Reader.h"
#include "../tests/dummy/Application.h"
#include "Logger.h"
#include <filesystem>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

namespace tkm::monitor
{

Reader::Reader(const std::shared_ptr<Options> options, Reader::Type type)
: Pollable("DummyReader")
, m_options(options)
, m_type(type)
{
  if (m_type == Reader::Type::INET) {
    if ((m_sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      throw std::runtime_error("Fail to create Connection socket");
    }
  } else {
    if ((m_sockFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
      throw std::runtime_error("Fail to create Connection socket");
    }
  }

  m_reader = std::make_unique<EnvelopeReader>(m_sockFd);
  m_writer = std::make_unique<EnvelopeWriter>(m_sockFd);

  lateSetup(
      [this]() {
        auto status = true;

        do {
          tkm::msg::Envelope envelope;

          // Read next message
          auto readStatus = readEnvelope(envelope);
          if (readStatus == IAsyncEnvelope::Status::Again) {
            return true;
          } else if (readStatus == IAsyncEnvelope::Status::Error) {
            logDebug() << "Read error";
            return false;
          } else if (readStatus == IAsyncEnvelope::Status::EndOfFile) {
            logDebug() << "Read end of file";
            return false;
          }

          // Check for valid origin
          if (envelope.origin() != tkm::msg::Envelope_Recipient_Monitor) {
            continue;
          }

          tkm::msg::monitor::Message msg;
          envelope.mesg().UnpackTo(&msg);

          switch (msg.type()) {
          case tkm::msg::monitor::Message_Type_SetSession: {
            msg.payload().UnpackTo(&m_sessionInfo);
            const std::string sessionName =
                "Collector." + std::to_string(getpid()) + "." + std::to_string(time(NULL));
            m_sessionInfo.set_name(sessionName);
            break;
          }
          case tkm::msg::monitor::Message_Type_Data: {
            tkm::msg::monitor::Data data;

            msg.payload().UnpackTo(&data);
            data.set_receive_time_sec(static_cast<uint64_t>(time(NULL)));

            switch (data.what()) {
            case tkm::msg::monitor::Data_What_ProcAcct: {
              data.payload().UnpackTo(&m_procAcct);
              m_procAcctCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_ProcInfo: {
              data.payload().UnpackTo(&m_procInfo);
              m_procInfoCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_ProcEvent: {
              data.payload().UnpackTo(&m_procEvent);
              m_procEventCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_ContextInfo: {
              data.payload().UnpackTo(&m_ctxInfo);
              m_ctxInfoCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_SysProcStat: {
              data.payload().UnpackTo(&m_sysProcStat);
              m_sysProcStatCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_SysProcMemInfo: {
              data.payload().UnpackTo(&m_sysProcMemInfo);
              m_sysProcMemInfoCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_SysProcDiskStats: {
              data.payload().UnpackTo(&m_sysProcDiskStats);
              m_sysProcDiskStatsCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_SysProcPressure: {
              data.payload().UnpackTo(&m_sysProcPressure);
              m_sysProcPressureCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_SysProcBuddyInfo: {
              data.payload().UnpackTo(&m_sysProcBuddyInfo);
              m_sysProcBuddyInfoCount++;
              break;
            }
            case tkm::msg::monitor::Data_What_SysProcWireless: {
              data.payload().UnpackTo(&m_sysProcWireless);
              m_sysProcWirelessCount++;
              break;
            }
            default:
              break;
            }
            break;
          }
          case tkm::msg::monitor::Message_Type_Status: {
            tkm::msg::monitor::Status s;
            msg.payload().UnpackTo(&s);

            break;
          }
          default:
            logError() << "Unknown response type";
            status = false;
            break;
          }
        } while (status);

        return status;
      },
      m_sockFd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // We are ready for events only after connect
  setPrepare([]() { return false; });
  // If the event is removed we stop the main application
  setFinalize([]() { logInfo() << "Device connection terminated"; });
}

void Reader::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(getShared());
  } else {
    App()->remEventSource(getShared());
  }
}

Reader::~Reader()
{
  logDebug() << "Reader " << getFD() << " destructed";
  if (getFD() > 0) {
    ::close(getFD());
  }
}

bool Reader::requestData(tkm::msg::collector::Request_Type type)
{
  tkm::msg::Envelope requestEnvelope;
  tkm::msg::collector::Request requestMessage;

  requestMessage.set_id("Test");
  requestMessage.set_type(type);
  requestEnvelope.mutable_mesg()->PackFrom(requestMessage);
  requestEnvelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
  requestEnvelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  return writeEnvelope(requestEnvelope);
}

auto Reader::connect() -> int
{
  int status = 0;

  if (m_type == Reader::Type::INET) {
    status = connectINET();
  } else {
    status = connectUNIX();
  }

  if (status < 0)
    return status;

  // Send descriptor
  tkm::msg::collector::Descriptor descriptor;
  descriptor.set_id("Reader");
  if (!sendCollectorDescriptor(getFD(), descriptor)) {
    logError() << "Failed to send descriptor";
    status = -1;
  }
  logDebug() << "Sent collector descriptor";

  // Request session
  if (status == 0) {
    tkm::msg::Envelope envelope;
    tkm::msg::collector::Request request;

    request.set_id("CreateSession");
    request.set_type(tkm::msg::collector::Request::Type::Request_Type_CreateSession);

    envelope.mutable_mesg()->PackFrom(request);
    envelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
    envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

    if (!writeEnvelope(envelope)) {
      status = -1;
    } else {
      logDebug() << "Session requested";
    }
  }

  return status;
}

auto Reader::connectINET() -> int
{
  int port = 0;

  std::string monitorAddress = m_options->getFor(Options::Key::TCPServerAddress);
  struct hostent *monitor = gethostbyname(monitorAddress.c_str());

  if (monitor == nullptr) {
    throw std::runtime_error("Invalid device address");
  }
  m_INETAddr.sin_family = AF_INET;
  memcpy(&m_INETAddr.sin_addr.s_addr, monitor->h_addr, (size_t) monitor->h_length);
  try {
    port = std::stoi(m_options->getFor(Options::Key::TCPServerPort));
  } catch (const std::exception &e) {
    port = std::stoi(tkmDefaults.getFor(Defaults::Default::TCPServerPort));
    logWarn() << "Cannot convert port number from config (using 3357): " << e.what();
  }
  m_INETAddr.sin_port = htons(static_cast<uint16_t>(port));

  if (::connect(m_sockFd, (struct sockaddr *) &m_INETAddr, sizeof(struct sockaddr_in)) == -1) {
    if (errno == EINPROGRESS) {
      fd_set wfds, efds;

      FD_ZERO(&wfds);
      FD_SET(m_sockFd, &wfds);

      FD_ZERO(&efds);
      FD_SET(m_sockFd, &efds);

      // We are going to use select to wait for the socket to connect
      struct timeval tv;
      tv.tv_sec = 3;
      tv.tv_usec = 0;

      auto ret = select(m_sockFd + 1, NULL, &wfds, &efds, &tv);
      if (ret == -1) {
        logError() << "Error Connecting";
        return -1;
      }
      if (ret == 0) {
        logError() << "Connection timeout";
        return -1;
      }
      if (!FD_ISSET(m_sockFd, &efds)) {
        int error = 0;
        socklen_t len = sizeof(error);

        if (getsockopt(m_sockFd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
          logError() << "Connection failed";
          return -1;
        }

        if (error != 0) {
          logError() << "Connection failed. Reason: " << strerror(error);
          return -1;
        }
      }
    } else {
      logError() << "Failed to connect to monitor: " << strerror(errno);
      return -1;
    }
  }

  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;

  if (setsockopt(m_sockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    logError() << "Failed to setsockopt SO_RCVTIMEO. Error: " << strerror(errno);
    return -1;
  }
  if (setsockopt(m_sockFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
    logError() << "Failed to setsockopt SO_SNDTIMEO. Error: " << strerror(errno);
    return -1;
  }

  int yes = 1;
  if (setsockopt(m_sockFd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) < 0) {
    logError() << "Failed to setsockopt SO_KEEPALIVE. Error: " << strerror(errno);
    return -1;
  }

  int idle = 1;
  if (setsockopt(m_sockFd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int)) < 0) {
    logError() << "Failed to setsockopt TCP_KEEPIDLE. Error: " << strerror(errno);
    return -1;
  }

  int interval = 2;
  if (setsockopt(m_sockFd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int)) < 0) {
    logError() << "Failed to setsockopt TCP_KEEPINTVL. Error: " << strerror(errno);
    return -1;
  }

  int maxpkt = 5;
  if (setsockopt(m_sockFd, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int)) < 0) {
    logError() << "Failed to setsockopt TCP_KEEPCNT. Error: " << strerror(errno);
    return -1;
  }

  // We are ready to process events
  logInfo() << "Connected to monitor";
  setPrepare([]() { return true; });

  return 0;
}

auto Reader::connectUNIX() -> int
{
  std::filesystem::path sockPath(m_options->getFor(Options::Key::UDSServerSocketPath));

  m_UNIXAddr.sun_family = AF_UNIX;
  strncpy(m_UNIXAddr.sun_path, sockPath.c_str(), sizeof(m_UNIXAddr.sun_path) - 1);

  if (!std::filesystem::exists(sockPath)) {
    throw std::runtime_error("Collector IPC socket not available");
  }

  if (::connect(m_sockFd, (struct sockaddr *) &m_UNIXAddr, sizeof(struct sockaddr_un)) == -1) {
    logError() << "Failed to connect to server";
    return -1;
  }

  // We are ready to process events
  logInfo() << "UDS reader connected to server";
  setPrepare([]() { return true; });

  return 0;
}

} // namespace tkm::monitor
