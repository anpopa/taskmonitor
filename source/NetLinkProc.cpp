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

#include <json/json.h>
#include <linux/cn_proc.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "JsonWriter.h"
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

            Json::Value head;
            Json::Value body;

            head["type"] = "proc";
            head["time"] = time(NULL);

            switch (nlcn_msg.proc_ev.what) {
            case proc_event::what::PROC_EVENT_NONE:
                logDebug() << "NetLinkProc Set mcast listen OK";
                break;
            case proc_event::what::PROC_EVENT_FORK:
                body["parent_id"] = nlcn_msg.proc_ev.event_data.fork.parent_pid;
                body["parent_tgid"] = nlcn_msg.proc_ev.event_data.fork.parent_tgid;
                body["child_pid"] = nlcn_msg.proc_ev.event_data.fork.child_pid;
                body["child_tgid"] = nlcn_msg.proc_ev.event_data.fork.child_tgid;
                head["fork"] = body;
                writeJsonStream() << head;
                break;
            case proc_event::what::PROC_EVENT_EXEC:
                body["process_pid"] = nlcn_msg.proc_ev.event_data.exec.process_pid;
                body["process_tgid"] = nlcn_msg.proc_ev.event_data.exec.process_tgid;
                head["exec"] = body;
                writeJsonStream() << head;
                TaskMonitor()->getRegistry()->addEntry(
                    nlcn_msg.proc_ev.event_data.exec.process_pid);
                break;
            case proc_event::what::PROC_EVENT_UID:
                body["process_pid"] = nlcn_msg.proc_ev.event_data.id.process_pid;
                body["process_tgid"] = nlcn_msg.proc_ev.event_data.id.process_tgid;
                body["ruid"] = nlcn_msg.proc_ev.event_data.id.r.ruid;
                body["euid"] = nlcn_msg.proc_ev.event_data.id.e.euid;
                head["uid"] = body;
                writeJsonStream() << head;
                break;
            case proc_event::what::PROC_EVENT_GID:
                body["process_pid"] = nlcn_msg.proc_ev.event_data.id.process_pid;
                body["process_tgid"] = nlcn_msg.proc_ev.event_data.id.process_tgid;
                body["rgid"] = nlcn_msg.proc_ev.event_data.id.r.rgid;
                body["egid"] = nlcn_msg.proc_ev.event_data.id.e.egid;
                head["gid"] = body;
                writeJsonStream() << head;
                break;
            case proc_event::what::PROC_EVENT_EXIT:
                body["process_pid"] = nlcn_msg.proc_ev.event_data.exit.process_pid;
                body["process_tgid"] = nlcn_msg.proc_ev.event_data.exit.process_tgid;
                body["exit_code"] = nlcn_msg.proc_ev.event_data.exit.exit_code;
                head["exit"] = body;
                writeJsonStream() << head;
                TaskMonitor()->getRegistry()->remEntry(
                    nlcn_msg.proc_ev.event_data.exec.process_pid);
                break;
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
