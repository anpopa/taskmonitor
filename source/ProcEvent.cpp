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
#include "Logger.h"
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
        __attribute__((aligned(NLMSG_ALIGNTO))) char
            msg_buf[sizeof(struct nlmsghdr) + sizeof(struct cn_msg) + sizeof(struct proc_event)]{};
        struct nlmsghdr *nl_hdr = reinterpret_cast<struct nlmsghdr *>(msg_buf);
        struct cn_msg *cn_msg = reinterpret_cast<struct cn_msg *>(NLMSG_DATA(nl_hdr));
        struct proc_event *proc_ev = reinterpret_cast<struct proc_event *>(&cn_msg->data[0]);
        ssize_t rc;

        rc = recv(m_sockFd, &msg_buf, sizeof(msg_buf), 0);
        if (rc == 0) {
          return true;
        } else if (rc == -1) {
          logError() << "NetLink process receive error: " << ::strerror(errno);
          return false;
        }

        switch (proc_ev->what) {
        case proc_event::what::PROC_EVENT_NONE:
          logDebug() << "ProcEvent Set mcast listen OK";
          break;
        case proc_event::what::PROC_EVENT_FORK: {
          logInfo() << "proc.event[fork]:"
                    << " parent_pid=" << proc_ev->event_data.fork.parent_pid
                    << " parent_tgid=" << proc_ev->event_data.fork.parent_tgid
                    << " child_pid=" << proc_ev->event_data.fork.child_pid
                    << " child_tgid=" << proc_ev->event_data.fork.child_tgid;
          m_eventData.set_fork_count(m_eventData.fork_count() + 1);

          // We only add a process entry in registry for processes
          if (proc_ev->event_data.fork.child_pid == proc_ev->event_data.fork.child_tgid) {
            if (m_options->getFor(Options::Key::UpdateOnProcEvent) ==
                tkmDefaults.valFor(Defaults::Val::True)) {
              App()->getProcRegistry()->addProcEntry(proc_ev->event_data.fork.child_tgid);
            }
          }
          break;
        }
        case proc_event::what::PROC_EVENT_EXEC: {
          logInfo() << "proc.event[exec]:"
                    << " process_pid=" << proc_ev->event_data.exec.process_pid
                    << " process_tgid=" << proc_ev->event_data.exec.process_tgid;
          m_eventData.set_exec_count(m_eventData.exec_count() + 1);
          if (m_options->getFor(Options::Key::UpdateOnProcEvent) ==
              tkmDefaults.valFor(Defaults::Val::True)) {
            App()->getProcRegistry()->updProcEntry(proc_ev->event_data.exec.process_pid);
          }
          break;
        }
        case proc_event::what::PROC_EVENT_UID: {
          logInfo() << "proc.event[uid]:"
                    << " process_pid=" << proc_ev->event_data.id.process_pid
                    << " process_tgid=" << proc_ev->event_data.id.process_tgid
                    << " ruid=" << proc_ev->event_data.id.r.ruid
                    << " euid=" << proc_ev->event_data.id.e.euid;
          m_eventData.set_uid_count(m_eventData.uid_count() + 1);
          break;
        }
        case proc_event::what::PROC_EVENT_GID: {
          logInfo() << "proc.event[gid]:"
                    << " process_pid=" << proc_ev->event_data.id.process_pid
                    << " process_tgid=" << proc_ev->event_data.id.process_tgid
                    << " rgid=" << proc_ev->event_data.id.r.rgid
                    << " egid=" << proc_ev->event_data.id.e.egid;
          m_eventData.set_gid_count(m_eventData.gid_count() + 1);
          break;
        }
        case proc_event::what::PROC_EVENT_EXIT: {
          logInfo() << "proc.event[exit]:"
                    << " process_pid=" << proc_ev->event_data.id.process_pid
                    << " process_tgid=" << proc_ev->event_data.id.process_tgid
                    << " exit_code=" << proc_ev->event_data.exit.exit_code;
          m_eventData.set_exit_count(m_eventData.exit_count() + 1);
          if (proc_ev->event_data.exit.process_pid == proc_ev->event_data.exit.process_tgid) {
            if (m_options->getFor(Options::Key::UpdateOnProcEvent) ==
                tkmDefaults.valFor(Defaults::Val::True)) {
              App()->getProcRegistry()->remProcEntry(proc_ev->event_data.exit.process_pid, true);
            }
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
  setFinalize([this]() {
    logInfo() << "ProcEvent kernel closed connection. Restarting module";
    App()->pushAction(Application::Action::ResetProcEvent);
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
  logWarn() << "ProcEvent module destructed";
}

auto ProcEvent::pushRequest(ProcEvent::Request &request) -> int
{
  return m_queue->push(request);
}

void ProcEvent::setEventSource(bool enabled)
{
  if (enabled) {
    // Main pollable events
    App()->addEventSource(getShared());

    // Request queue events
    App()->addEventSource(m_queue);

    // Start monitoring
    startMonitoring();
  } else {
    // Request queue events
    App()->remEventSource(m_queue);

    // Main pollable events
    App()->remEventSource(getShared());
  }
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
  __attribute__((aligned(NLMSG_ALIGNTO))) char
      msg_buf[sizeof(struct nlmsghdr) + sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op)]{};
  struct nlmsghdr *nl_hdr = reinterpret_cast<struct nlmsghdr *>(msg_buf);
  struct cn_msg *cn_msg = reinterpret_cast<struct cn_msg *>(NLMSG_DATA(nl_hdr));
  enum proc_cn_mcast_op *cn_mcast = reinterpret_cast<enum proc_cn_mcast_op *>(&cn_msg->data[0]);

  nl_hdr->nlmsg_len = sizeof(msg_buf);
  nl_hdr->nlmsg_pid = static_cast<uint32_t>(getpid());
  nl_hdr->nlmsg_type = NLMSG_DONE;

  cn_msg->id.idx = CN_IDX_PROC;
  cn_msg->id.val = CN_VAL_PROC;
  cn_msg->len = sizeof(enum proc_cn_mcast_op);

  *cn_mcast = PROC_CN_MCAST_LISTEN;

  if (send(m_sockFd, &msg_buf, sizeof(msg_buf), 0) == -1) {
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
