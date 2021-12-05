/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Alin Popa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
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

namespace fs = std::filesystem;
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

            // see linux/cn_proc.h
            enum what {
                PROC_EVENT_NONE = 0x00000000,
                PROC_EVENT_FORK = 0x00000001,
                PROC_EVENT_EXEC = 0x00000002,
                PROC_EVENT_UID = 0x00000004,
                PROC_EVENT_GID = 0x00000040,
                PROC_EVENT_SID = 0x00000080,
                PROC_EVENT_PTRACE = 0x00000100,
                PROC_EVENT_COMM = 0x00000200,
                PROC_EVENT_COREDUMP = 0x40000000,
                PROC_EVENT_EXIT = 0x80000000
            };

            rc = recv(m_sockFd, &nlcn_msg, sizeof(nlcn_msg), 0);
            if (rc == 0) {
                return true;
            } else if (rc == -1) {
                logError() << "Netlink process receive error";
                return false;
            }

            switch (nlcn_msg.proc_ev.what) {
            case PROC_EVENT_NONE:
                logInfo() << "MON::PROC::NONE Set mcast listen OK";
                break;
            case PROC_EVENT_FORK:
                logInfo() << "MON::PROC::FORK ParentTID="
                          << nlcn_msg.proc_ev.event_data.fork.parent_pid << " "
                          << "ParentPID=" << nlcn_msg.proc_ev.event_data.fork.parent_tgid << " "
                          << "ChildTID=" << nlcn_msg.proc_ev.event_data.fork.child_tgid << " "
                          << "ChildPID=" << nlcn_msg.proc_ev.event_data.fork.child_pid;
                break;
            case PROC_EVENT_EXEC:
                logInfo() << "MON::PROC::EXEC TID=" << nlcn_msg.proc_ev.event_data.exec.process_tgid
                          << " "
                          << "PID=" << nlcn_msg.proc_ev.event_data.exec.process_pid;
                break;
            case PROC_EVENT_UID:
                logInfo() << "MON::PROC::UID TID=" << nlcn_msg.proc_ev.event_data.id.process_tgid
                          << " "
                          << "PID=" << nlcn_msg.proc_ev.event_data.id.process_pid << " "
                          << "From=" << nlcn_msg.proc_ev.event_data.id.r.ruid << " "
                          << "To=" << nlcn_msg.proc_ev.event_data.id.e.euid;
                break;
            case PROC_EVENT_GID:
                logInfo() << "MON::PROC::GID TID=" << nlcn_msg.proc_ev.event_data.id.process_tgid
                          << " "
                          << "PID=" << nlcn_msg.proc_ev.event_data.id.process_pid << " "
                          << "From=" << nlcn_msg.proc_ev.event_data.id.r.rgid << " "
                          << "To=" << nlcn_msg.proc_ev.event_data.id.e.egid;
                break;
            case PROC_EVENT_EXIT:
                logInfo() << "MON::PROC::EXIT TID=" << nlcn_msg.proc_ev.event_data.exit.process_tgid
                          << " "
                          << "PID=" << nlcn_msg.proc_ev.event_data.exit.process_pid << " "
                          << "ExitCode=" << nlcn_msg.proc_ev.event_data.exit.exit_code;
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
