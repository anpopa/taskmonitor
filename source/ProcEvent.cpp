/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcEvent Class
 * @details   Monitor system process events using netlink interfaces
 *-
 */

#include <linux/cn_proc.h>
#include <linux/connector.h>
#include <netdb.h>
#include <unistd.h>

#include "Application.h"
#include "ProcEvent.h"

namespace tkm::monitor
{

static bool doCollectAndSend(const std::shared_ptr<ProcEvent> mgr, const ProcEvent::Request &rq);

ProcEvent::ProcEvent(const std::shared_ptr<Options> options)
: Pollable("ProcEvent")
, m_options(options)
{
  int txBufferSize, rxBufferSize;

  try {
    txBufferSize = std::stoi(m_options->getFor(Options::Key::TxBufferSize));
    rxBufferSize = std::stoi(m_options->getFor(Options::Key::RxBufferSize));
  } catch (std::exception &e) {
    throw std::runtime_error("Invalid TX/RX buffer size in configuration");
  }

  if ((m_sockFd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR)) == -1) {
    throw std::runtime_error("Fail to create netlink socket");
  }

  if (setsockopt(m_sockFd, SOL_SOCKET, SO_RCVBUF, &rxBufferSize, sizeof(rxBufferSize)) == -1) {
    throw std::runtime_error("Fail to set netlink rx socket buffer size");
  }

  if (setsockopt(m_sockFd, SOL_SOCKET, SO_SNDBUF, &txBufferSize, sizeof(txBufferSize)) == -1) {
    throw std::runtime_error("Fail to set netlink tx socket buffer size");
  }

  m_addr.nl_family = AF_NETLINK;
  m_addr.nl_groups = CN_IDX_PROC;
  m_addr.nl_pid = static_cast<unsigned int>(getpid());

  if (bind(m_sockFd, (struct sockaddr *) &m_addr, sizeof(m_addr)) == -1) {
    throw std::runtime_error("Fail to bind netlink socket");
  }

  lateSetup(
      [this]() {
        struct __attribute__((aligned(NLMSG_ALIGNTO))) {
          struct nlmsghdr nl_hdr;
          struct __attribute__((__packed__)) {
            struct cn_msg cn_msg;
            struct proc_event proc_ev;
          };
        } nlcn_msg;
        ssize_t rc;

        rc = recv(m_sockFd, &nlcn_msg, sizeof(nlcn_msg), 0);
        if (rc == 0) {
          return true;
        } else if (rc == -1) {
          logError() << "NetLink process receive error: " << ::strerror(errno);
          return false;
        }

        // Fill common data
        tkm::msg::monitor::Data data;
        tkm::msg::monitor::ProcEvent procEvent;
        data.set_what(tkm::msg::monitor::Data_What_ProcEvent);

        struct timespec currentTime;
        clock_gettime(CLOCK_REALTIME, &currentTime);
        data.set_system_time_sec(static_cast<uint64_t>(currentTime.tv_sec));
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        data.set_monotonic_time_sec(static_cast<uint64_t>(currentTime.tv_sec));

        switch (nlcn_msg.proc_ev.what) {
        case proc_event::what::PROC_EVENT_NONE:
          logDebug() << "ProcEvent Set mcast listen OK";
          break;
        case proc_event::what::PROC_EVENT_FORK: {
          logInfo() << "proc.event[fork]:"
                    << " parent_pid=" << nlcn_msg.proc_ev.event_data.fork.parent_pid
                    << " parent_tgid=" << nlcn_msg.proc_ev.event_data.fork.parent_tgid
                    << " child_pid=" << nlcn_msg.proc_ev.event_data.fork.child_pid
                    << " child_tgid=" << nlcn_msg.proc_ev.event_data.fork.child_tgid;
          m_eventData.set_fork_count(m_eventData.fork_count() + 1);

          // We only add a process entry in registry for processes
          if (nlcn_msg.proc_ev.event_data.fork.child_pid ==
              nlcn_msg.proc_ev.event_data.fork.child_tgid) {
            App()->getProcRegistry()->addProcEntry(nlcn_msg.proc_ev.event_data.fork.child_tgid);
          }
          break;
        }
        case proc_event::what::PROC_EVENT_EXEC: {
          logInfo() << "proc.event[exec]:"
                    << " process_pid=" << nlcn_msg.proc_ev.event_data.exec.process_pid
                    << " process_tgid=" << nlcn_msg.proc_ev.event_data.exec.process_tgid;
          m_eventData.set_exec_count(m_eventData.exec_count() + 1);
          App()->getProcRegistry()->updProcEntry(nlcn_msg.proc_ev.event_data.exec.process_pid);
          break;
        }
        case proc_event::what::PROC_EVENT_UID: {
          logInfo() << "proc.event[uid]:"
                    << " process_pid=" << nlcn_msg.proc_ev.event_data.id.process_pid
                    << " process_tgid=" << nlcn_msg.proc_ev.event_data.id.process_tgid
                    << " ruid=" << nlcn_msg.proc_ev.event_data.id.r.ruid
                    << " euid=" << nlcn_msg.proc_ev.event_data.id.e.euid;
          m_eventData.set_uid_count(m_eventData.uid_count() + 1);
          break;
        }
        case proc_event::what::PROC_EVENT_GID: {
          logInfo() << "proc.event[gid]:"
                    << " process_pid=" << nlcn_msg.proc_ev.event_data.id.process_pid
                    << " process_tgid=" << nlcn_msg.proc_ev.event_data.id.process_tgid
                    << " rgid=" << nlcn_msg.proc_ev.event_data.id.r.rgid
                    << " egid=" << nlcn_msg.proc_ev.event_data.id.e.egid;
          m_eventData.set_gid_count(m_eventData.gid_count() + 1);
          break;
        }
        case proc_event::what::PROC_EVENT_EXIT: {
          logInfo() << "proc.event[exit]:"
                    << " process_pid=" << nlcn_msg.proc_ev.event_data.id.process_pid
                    << " process_tgid=" << nlcn_msg.proc_ev.event_data.id.process_tgid
                    << " exit_code=" << nlcn_msg.proc_ev.event_data.exit.exit_code;
          m_eventData.set_exit_count(m_eventData.exit_count() + 1);
          if (nlcn_msg.proc_ev.event_data.exit.process_pid ==
              nlcn_msg.proc_ev.event_data.exit.process_tgid) {
            App()->getProcRegistry()->remProcEntry(nlcn_msg.proc_ev.event_data.exit.process_pid,
                                                   true);
          }
          break;
        }
        default:
          break;
        }

        return true;
      },
      m_sockFd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // If the event is removed we stop the main application
  setFinalize([]() {
    logInfo() << "Server closed connection. Terminate";
    App()->stop();
  });

  m_queue = std::make_shared<AsyncQueue<ProcEvent::Request>>(
      "ProcEventQueue", [this](const Request &request) { return requestHandler(request); });
}

ProcEvent::~ProcEvent()
{
  if (m_sockFd != -1) {
    ::close(m_sockFd);
    m_sockFd = -1;
  }
}

auto ProcEvent::pushRequest(ProcEvent::Request &request) -> int
{
  return m_queue->push(request);
}

void ProcEvent::enableEvents()
{
  // Main pollable events
  App()->addEventSource(getShared());

  // Request queue events
  App()->addEventSource(m_queue);

  // Start monitoring
  startMonitoring();
}

auto ProcEvent::requestHandler(const Request &request) -> bool
{
  switch (request.action) {
  case ProcEvent::Action::CollectAndSend:
    return doCollectAndSend(getShared(), request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

void ProcEvent::startMonitoring(void)
{
  struct __attribute__((aligned(NLMSG_ALIGNTO))) {
    struct nlmsghdr nl_hdr;
    struct __attribute__((__packed__)) {
      struct cn_msg cn_msg;
      enum proc_cn_mcast_op cn_mcast;
    };
  } nlcn_msg;

  memset(&nlcn_msg, 0, sizeof(nlcn_msg));
  nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
  nlcn_msg.nl_hdr.nlmsg_pid = static_cast<uint32_t>(getpid());
  nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

  nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
  nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
  nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);
  nlcn_msg.cn_mcast = PROC_CN_MCAST_LISTEN;

  if (send(m_sockFd, &nlcn_msg, sizeof(nlcn_msg), 0) == -1) {
    logError() << "Netlink send error";
  }
}

static bool doCollectAndSend(const std::shared_ptr<ProcEvent> mgr, const ProcEvent::Request &rq)
{
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_ProcEvent);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(static_cast<uint64_t>(currentTime.tv_sec));
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(static_cast<uint64_t>(currentTime.tv_sec));

  data.mutable_payload()->PackFrom(mgr->getProcEventData());

  rq.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
