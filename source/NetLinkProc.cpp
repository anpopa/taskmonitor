/*
 * SPDX license identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2021 Alin Popa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * \author Alin Popa <alin.popa@fxdata.ro>
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

            switch (nlcn_msg.proc_ev.what) {
            case proc_event::what::PROC_EVENT_NONE:
                logInfo() << "MON::PROC::NONE Set mcast listen OK";
                break;
            case proc_event::what::PROC_EVENT_FORK:
                logInfo() << "MON::PROC::FORK ParentTID="
                          << nlcn_msg.proc_ev.event_data.fork.parent_pid
                          << " ParentPID=" << nlcn_msg.proc_ev.event_data.fork.parent_tgid
                          << " ChildTID=" << nlcn_msg.proc_ev.event_data.fork.child_tgid
                          << " ChildPID=" << nlcn_msg.proc_ev.event_data.fork.child_pid;
                break;
            case proc_event::what::PROC_EVENT_EXEC:
                logInfo() << "MON::PROC::EXEC TID=" << nlcn_msg.proc_ev.event_data.exec.process_tgid
                          << " PID=" << nlcn_msg.proc_ev.event_data.exec.process_pid;
                TaskMonitor()->getRegistry()->addEntry(
                    nlcn_msg.proc_ev.event_data.exec.process_pid);
                break;
            case proc_event::what::PROC_EVENT_UID:
                logInfo() << "MON::PROC::UID TID=" << nlcn_msg.proc_ev.event_data.id.process_tgid
                          << " PID=" << nlcn_msg.proc_ev.event_data.id.process_pid
                          << " From=" << nlcn_msg.proc_ev.event_data.id.r.ruid
                          << " To=" << nlcn_msg.proc_ev.event_data.id.e.euid;
                break;
            case proc_event::what::PROC_EVENT_GID:
                logInfo() << "MON::PROC::GID TID=" << nlcn_msg.proc_ev.event_data.id.process_tgid
                          << " PID=" << nlcn_msg.proc_ev.event_data.id.process_pid
                          << " From=" << nlcn_msg.proc_ev.event_data.id.r.rgid
                          << " To=" << nlcn_msg.proc_ev.event_data.id.e.egid;
                break;
            case proc_event::what::PROC_EVENT_EXIT:
                logInfo() << "MON::PROC::EXIT TID=" << nlcn_msg.proc_ev.event_data.exit.process_tgid
                          << " PID=" << nlcn_msg.proc_ev.event_data.exit.process_pid
                          << " ExitCode=" << nlcn_msg.proc_ev.event_data.exit.exit_code;
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
