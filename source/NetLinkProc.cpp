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

#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>

#include <netlink/attr.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "NetLinkProc.h"

namespace fs = std::filesystem;
using std::shared_ptr;
using std::string;

int callbackProcessMessage(struct nl_msg *nlmsg, void *arg)
{
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg;
            struct proc_event proc_ev;
        };
    } nlcn_msg;
    struct nlmsghdr *nlhdr;

    nlhdr = nlmsg_hdr(nlmsg);
    memcpy(&nlcn_msg, nlmsg_data(nlhdr), sizeof(nlcn_msg));

	enum what {
		/* Use successive bits so the enums can be used to record
		 * sets of events as well
		 */
		PROC_EVENT_NONE = 0x00000000,
		PROC_EVENT_FORK = 0x00000001,
		PROC_EVENT_EXEC = 0x00000002,
		PROC_EVENT_UID  = 0x00000004,
		PROC_EVENT_GID  = 0x00000040,
		PROC_EVENT_SID  = 0x00000080,
		PROC_EVENT_PTRACE = 0x00000100,
		PROC_EVENT_COMM = 0x00000200,
		/* "next" should be 0x00000400 */
		/* "last" is the last process event: exit,
		 * while "next to last" is coredumping event */
		PROC_EVENT_COREDUMP = 0x40000000,
		PROC_EVENT_EXIT = 0x80000000
	};

    switch (nlcn_msg.proc_ev.what) {
            case PROC_EVENT_NONE:
                printf("set mcast listen ok\n");
                break;
            case PROC_EVENT_FORK:
                printf("fork: parent tid=%d pid=%d -> child tid=%d pid=%d\n",
                        nlcn_msg.proc_ev.event_data.fork.parent_pid,
                        nlcn_msg.proc_ev.event_data.fork.parent_tgid,
                        nlcn_msg.proc_ev.event_data.fork.child_pid,
                        nlcn_msg.proc_ev.event_data.fork.child_tgid);
                break;
            case PROC_EVENT_EXEC:
                printf("exec: tid=%d pid=%d\n",
                        nlcn_msg.proc_ev.event_data.exec.process_pid,
                        nlcn_msg.proc_ev.event_data.exec.process_tgid);
                break;
            case PROC_EVENT_UID:
                printf("uid change: tid=%d pid=%d from %d to %d\n",
                        nlcn_msg.proc_ev.event_data.id.process_pid,
                        nlcn_msg.proc_ev.event_data.id.process_tgid,
                        nlcn_msg.proc_ev.event_data.id.r.ruid,
                        nlcn_msg.proc_ev.event_data.id.e.euid);
                break;
            case PROC_EVENT_GID:
                printf("gid change: tid=%d pid=%d from %d to %d\n",
                        nlcn_msg.proc_ev.event_data.id.process_pid,
                        nlcn_msg.proc_ev.event_data.id.process_tgid,
                        nlcn_msg.proc_ev.event_data.id.r.rgid,
                        nlcn_msg.proc_ev.event_data.id.e.egid);
                break;
            case PROC_EVENT_EXIT:
                printf("exit: tid=%d pid=%d exit_code=%d\n",
                        nlcn_msg.proc_ev.event_data.exit.process_pid,
                        nlcn_msg.proc_ev.event_data.exit.process_tgid,
                        nlcn_msg.proc_ev.event_data.exit.exit_code);
                break;
            default:
                break;
        }

    return 0;
}

namespace tkm::monitor
{

NetLinkProc::NetLinkProc(std::shared_ptr<Options> &options)
: Pollable("NetLinkProc")
, m_options(options)
{
    int err = NLE_SUCCESS;

    if ((m_nlSock = nl_socket_alloc()) == nullptr) {
        throw std::runtime_error("Fail to create netlink socket");
    }

    if ((err = nl_connect(m_nlSock, NETLINK_CONNECTOR)) < 0) {
        logError() << "Error connecting: " << nl_geterror(err);
        throw std::runtime_error("Fail to connect netlink socket");
    }

    if ((err = nl_socket_set_nonblocking(m_nlSock)) < 0) {
        logError() << "Error setting socket nonblocking: " << nl_geterror(err);
        throw std::runtime_error("Fail to set nonblocking netlink socket");
    }

    if ((m_sockFd = nl_socket_get_fd(m_nlSock)) < 0) {
        throw std::runtime_error("Fail to get netlink socket");
    }

    nl_socket_add_memberships(m_nlSock, CN_IDX_PROC, 0);
    nl_socket_disable_seq_check(m_nlSock);

    if ((err = nl_socket_modify_cb(m_nlSock, NL_CB_MSG_IN, NL_CB_CUSTOM, callbackProcessMessage, this))
        < 0) {
        logError() << "Error setting socket cb: " << nl_geterror(err);
        throw std::runtime_error("Fail to set message callback");
    }

    lateSetup(
        [this]() {
            int err = NLE_SUCCESS;

            if ((err = nl_recvmsgs_default(m_nlSock)) < 0) {
                if ((err != -NLE_AGAIN) && (err != -NLE_BUSY)) {
                    logError() << "Error receiving message: " << nl_geterror(err);
                    return false;
                }
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
    if (m_nlSock != nullptr) {
        nl_close(m_nlSock);
        nl_socket_free(m_nlSock);
    }
}

auto NetLinkProc::startProcMonitoring(void) -> int
{
    struct nlmsghdr *hdr = nullptr;
    struct nl_msg *msg = nullptr;
    struct cn_msg cn_msg;
    int err = NLE_SUCCESS;

    logDebug() << "Request process monitoring";

    if (!(msg = nlmsg_alloc())) {
        logError() << "Failed to alloc message: " << nl_geterror(err);
        return -1;
    }

    memset(&cn_msg, 0, sizeof(cn_msg));
    cn_msg.id.idx = CN_IDX_PROC;
    cn_msg.id.val = CN_VAL_PROC;

    if (!(hdr = static_cast<struct nlmsghdr *>(nlmsg_put(msg,
                                                         NL_AUTO_PID,
                                                         NL_AUTO_SEQ,
                                                         NLMSG_DONE,
                                                         sizeof(cn_msg),
                                                         NLM_F_REQUEST)))) {
        logError() << "Error setting message header";
        nlmsg_free(msg);
        return -1;
    }


    memcpy(nlmsg_data(hdr), &cn_msg, sizeof(cn_msg));

    if ((err = nl_send_sync(m_nlSock, msg)) < 0) {
        logError() << "Error sending message: " << nl_geterror(err);
        return -1;
    } // nl_send_sync free the msg

    return 0;
}

} // namespace tkm::monitor
