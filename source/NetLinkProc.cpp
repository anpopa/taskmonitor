/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     NetLinkProc Class
 * @details   Monitor system process events using netlink interfaces
 *-
 */

#include <csignal>
#include <filesystem>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/cn_proc.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "NetLinkProc.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

NetLinkProc::NetLinkProc(std::shared_ptr<Options> &options)
: Pollable("NetLinkProc")
, m_options(options)
{
  if ((m_sockFd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR)) == -1) {
    throw std::runtime_error("Fail to create netlink socket");
  }

  m_addr.nl_family = AF_NETLINK;
  m_addr.nl_groups = CN_IDX_PROC;
  m_addr.nl_pid = getpid();

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
        int rc;

        rc = recv(m_sockFd, &nlcn_msg, sizeof(nlcn_msg), 0);
        if (rc == 0) {
          return true;
        } else if (rc == -1) {
          logError() << "Netlink process receive error";
          return false;
        }

        // Fill common data
        tkm::msg::server::Data data;
        tkm::msg::server::ProcEvent procEvent;
        data.set_what(tkm::msg::server::Data_What_ProcEvent);
        data.set_timestamp(time(NULL));

        switch (nlcn_msg.proc_ev.what) {
        case proc_event::what::PROC_EVENT_NONE:
          logDebug() << "NetLinkProc Set mcast listen OK";
          break;
        case proc_event::what::PROC_EVENT_FORK: {
          tkm::msg::server::ProcEventFork forkData;

          forkData.set_parent_pid(nlcn_msg.proc_ev.event_data.fork.parent_pid);
          forkData.set_parent_tgid(nlcn_msg.proc_ev.event_data.fork.parent_tgid);
          forkData.set_child_pid(nlcn_msg.proc_ev.event_data.fork.child_pid);
          forkData.set_child_tgid(nlcn_msg.proc_ev.event_data.fork.child_tgid);

          procEvent.set_what(tkm::msg::server::ProcEvent_What_Fork);
          procEvent.mutable_data()->PackFrom(forkData);
          data.mutable_payload()->PackFrom(procEvent);

          TaskMonitor()->getNetServer()->sendData(data);
          break;
        }
        case proc_event::what::PROC_EVENT_EXEC: {
          tkm::msg::server::ProcEventExec execData;

          execData.set_process_pid(nlcn_msg.proc_ev.event_data.exec.process_pid);
          execData.set_process_tgid(nlcn_msg.proc_ev.event_data.exec.process_tgid);

          procEvent.set_what(tkm::msg::server::ProcEvent_What_Exec);
          procEvent.mutable_data()->PackFrom(execData);
          data.mutable_payload()->PackFrom(procEvent);

          TaskMonitor()->getNetServer()->sendData(data);
          TaskMonitor()->getRegistry()->addEntry(nlcn_msg.proc_ev.event_data.exec.process_pid);
          break;
        }
        case proc_event::what::PROC_EVENT_UID: {
          tkm::msg::server::ProcEventUID uidData;

          uidData.set_process_pid(nlcn_msg.proc_ev.event_data.id.process_pid);
          uidData.set_process_tgid(nlcn_msg.proc_ev.event_data.id.process_tgid);
          uidData.set_ruid(nlcn_msg.proc_ev.event_data.id.r.ruid);
          uidData.set_euid(nlcn_msg.proc_ev.event_data.id.e.euid);

          procEvent.set_what(tkm::msg::server::ProcEvent_What_UID);
          procEvent.mutable_data()->PackFrom(uidData);
          data.mutable_payload()->PackFrom(procEvent);

          TaskMonitor()->getNetServer()->sendData(data);
          break;
        }
        case proc_event::what::PROC_EVENT_GID: {
          tkm::msg::server::ProcEventGID gidData;

          gidData.set_process_pid(nlcn_msg.proc_ev.event_data.id.process_pid);
          gidData.set_process_tgid(nlcn_msg.proc_ev.event_data.id.process_tgid);
          gidData.set_rgid(nlcn_msg.proc_ev.event_data.id.r.rgid);
          gidData.set_egid(nlcn_msg.proc_ev.event_data.id.e.egid);

          procEvent.set_what(tkm::msg::server::ProcEvent_What_GID);
          procEvent.mutable_data()->PackFrom(gidData);
          data.mutable_payload()->PackFrom(procEvent);

          TaskMonitor()->getNetServer()->sendData(data);
          break;
        }
        case proc_event::what::PROC_EVENT_EXIT: {
          tkm::msg::server::ProcEventExit exitData;

          exitData.set_process_pid(nlcn_msg.proc_ev.event_data.id.process_pid);
          exitData.set_process_tgid(nlcn_msg.proc_ev.event_data.id.process_tgid);
          exitData.set_exit_code(nlcn_msg.proc_ev.event_data.exit.exit_code);

          procEvent.set_what(tkm::msg::server::ProcEvent_What_Exit);
          procEvent.mutable_data()->PackFrom(exitData);
          data.mutable_payload()->PackFrom(procEvent);

          TaskMonitor()->getNetServer()->sendData(data);
          TaskMonitor()->getRegistry()->remEntry(nlcn_msg.proc_ev.event_data.exit.process_pid);
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
    ::raise(SIGTERM);
  });
}

void NetLinkProc::enableEvents()
{
  TaskMonitor()->addEventSource(getShared());
}

NetLinkProc::~NetLinkProc()
{
  if (m_sockFd != -1) {
    ::close(m_sockFd);
  }
}

auto NetLinkProc::startProcMonitoring(void) -> int
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
  nlcn_msg.nl_hdr.nlmsg_pid = getpid();
  nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

  nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
  nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
  nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);
  nlcn_msg.cn_mcast = PROC_CN_MCAST_LISTEN;

  if (send(m_sockFd, &nlcn_msg, sizeof(nlcn_msg), 0) == -1) {
    logError() << "Netlink send error";
    return -1;
  }

  return 0;
}

} // namespace tkm::monitor
