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

#include <linux/taskstats.h>
#include <netlink/attr.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "NetLinkStats.h"

namespace fs = std::filesystem;
using std::shared_ptr;
using std::string;

#define average_ms(t, c) (t / 1000000ULL / (c ? c : 1))

static void printDelayAcct(struct taskstats *t)
{
    logInfo() << "MON::COMMON[" << t->ac_pid << "] "
              << "Command=" << t->ac_comm << " "
              << "UID=" << t->ac_uid << " "
              << "GID=" << t->ac_gid << " "
              << "PID=" << t->ac_pid << " "
              << "PPID=" << t->ac_ppid << " "
              << "UserCPUTime=" << t->ac_utime << " "
              << "SystemCPUTime=" << t->ac_stime;

    logInfo() << "MON::CPU[" << t->ac_pid << "] "
              << "Count=" << t->cpu_count << " "
              << "RealTotal=" << t->cpu_run_real_total << "ns "
              << "VirtualTotal=" << t->cpu_run_virtual_total << "ns "
              << "DelayTotal=" << t->cpu_delay_total << " "
              << "DelayAverage=" << average_ms((double) t->cpu_delay_total, t->cpu_count);

    logInfo() << "MON::MEMORY[" << t->ac_pid << "] "
              << "CoreMem=" << t->coremem << "MB-usec "
              << "VirtMem=" << t->virtmem << "MB-usec "
              << "HiWaterRSS=" << t->hiwater_rss << "KBytes "
              << "HiWaterVM=" << t->hiwater_vm << "KBytes";

    logInfo() << "MON::CONTEXT[" << t->ac_pid << "] "
              << "Voluntary=" << t->nvcsw << " "
              << "NonVoluntary=" << t->nivcsw;

    logInfo() << "MON::IO[" << t->ac_pid << "] "
              << "Count=" << t->blkio_count << " "
              << "DelayTotal=" << t->blkio_delay_total << " "
              << "DelayAverage=" << average_ms(t->blkio_delay_total, t->blkio_count);

    logInfo() << "MON::SWAP[" << t->ac_pid << "] "
              << "Count=" << t->swapin_count << " "
              << "DelayTotal=" << t->swapin_delay_total << " "
              << "DelayAverage=" << average_ms(t->swapin_delay_total, t->swapin_count);

    logInfo() << "MON::RECLAIM[" << t->ac_pid << "] "
              << "Count=" << t->freepages_count << " "
              << "DelayTotal=" << t->freepages_delay_total << " "
              << "DelayAverage=" << average_ms(t->freepages_delay_total, t->freepages_count);

    logInfo() << "MON::THRASHING[" << t->ac_pid << "] "
              << "Count=" << t->thrashing_count << " "
              << "DelayTotal=" << t->thrashing_delay_total << " "
              << "DelayAverage=" << average_ms(t->thrashing_delay_total, t->thrashing_count);
}

int callbackStatisticsMessage(struct nl_msg *nlmsg, void *arg)
{

    struct nlmsghdr *nlhdr;
    struct nlattr *nlattrs[TASKSTATS_TYPE_MAX + 1];
    struct nlattr *nlattr;
    struct taskstats *stats;
    int rem, answer;

    nlhdr = nlmsg_hdr(nlmsg);
    if ((answer = genlmsg_parse(nlhdr, 0, nlattrs, TASKSTATS_TYPE_MAX, NULL)) < 0) {
        logError() << "Error parsing msg";
        return -1;
    }

    if ((nlattr = nlattrs[TASKSTATS_TYPE_AGGR_PID]) || (nlattr = nlattrs[TASKSTATS_TYPE_TGID])) {
        stats = static_cast<struct taskstats *>(
            nla_data(nla_next(static_cast<struct nlattr *>(nla_data(nlattr)), &rem)));
        printDelayAcct(stats);
    } else {
        logError() << "Unknown attribute format received";
        return -1;
    }

    return 0;
}

namespace tkm::monitor
{

NetLinkStats::NetLinkStats(std::shared_ptr<Options> &options)
: Pollable("NetLinkStats")
, m_options(options)
{
    int err = NLE_SUCCESS;

    if ((m_nlSock = nl_socket_alloc()) == nullptr) {
        throw std::runtime_error("Fail to create netlink socket");
    }

    if ((err = nl_connect(m_nlSock, NETLINK_GENERIC)) < 0) {
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

    if ((m_nlFamily = genl_ctrl_resolve(m_nlSock, TASKSTATS_GENL_NAME)) == 0) {
        logError() << "Error retrieving family id: " << nl_geterror(err);
        throw std::runtime_error("Fail to retirve family id");
    }

    if ((err = nl_socket_modify_cb(
             m_nlSock, NL_CB_VALID, NL_CB_CUSTOM, callbackStatisticsMessage, this))
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

void NetLinkStats::enableEvents()
{
    TaskMonitor()->addEventSource(getShared());
}

NetLinkStats::~NetLinkStats()
{
    if (m_nlSock != nullptr) {
        nl_close(m_nlSock);
        nl_socket_free(m_nlSock);
    }
}

auto NetLinkStats::requestTaskAcct(int pid) -> int
{
    struct nlmsghdr *hdr = nullptr;
    struct nl_msg *msg = nullptr;
    int err = NLE_SUCCESS;

    logDebug() << "Request task accounting for pid " << pid;

    if (!(msg = nlmsg_alloc())) {
        logError() << "Failed to alloc message: " << nl_geterror(err);
        return -1;
    }

    if (!(hdr = static_cast<struct nlmsghdr *>(genlmsg_put(msg,
                                                           NL_AUTO_PID,
                                                           NL_AUTO_SEQ,
                                                           m_nlFamily,
                                                           0,
                                                           NLM_F_REQUEST,
                                                           TASKSTATS_CMD_GET,
                                                           TASKSTATS_VERSION)))) {
        logError() << "Error setting message header";
        nlmsg_free(msg);
        return -1;
    }

    if ((err = nla_put_u32(msg, TASKSTATS_CMD_ATTR_PID, pid)) < 0) {
        logError() << "Error setting attribute: " << nl_geterror(err);
        nlmsg_free(msg);
        return -1;
    }

    if ((err = nl_send_sync(m_nlSock, msg)) < 0) {
        logError() << "Error sending message: " << nl_geterror(err);
        return -1;
    } // nl_send_sync free the msg

    return 0;
}

} // namespace tkm::monitor
