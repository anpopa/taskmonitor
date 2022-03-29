/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     NetLinkStats Class
 * @details   Collect process statistics for each ProcEntry
 *-
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

using std::shared_ptr;
using std::string;

#define average_ms(t, c) (t / 1000000ULL / (c ? c : 1))

static void processDelayAcct(struct taskstats *t)
{
    auto entry = TaskMonitor()->getRegistry()->getEntry(t->ac_pid);

    if (entry == nullptr) {
        logError() << "Stat entry with PID " << t->ac_pid << " not in registry";
        return;
    }

    tkm::msg::server::Data data;
    tkm::msg::server::ProcAcct acct;

    data.set_what(tkm::msg::server::Data_What_ProcAcct);
    data.set_timestamp(time(NULL));

    acct.set_ac_comm(t->ac_comm);
    acct.set_ac_uid(t->ac_uid);
    acct.set_ac_gid(t->ac_gid);
    acct.set_ac_pid(t->ac_pid);
    acct.set_ac_ppid(t->ac_ppid);
    acct.set_ac_utime(t->ac_utime);
    acct.set_ac_stime(t->ac_stime);
    acct.set_user_cpu_percent(entry->getUserCPUPercent(t->ac_utime));
    acct.set_sys_cpu_percent(entry->getSystemCPUPercent(t->ac_stime));

    acct.mutable_cpu()->set_cpu_count(t->cpu_count);
    acct.mutable_cpu()->set_cpu_run_real_total(t->cpu_run_real_total);
    acct.mutable_cpu()->set_cpu_run_virtual_total(t->cpu_run_virtual_total);
    acct.mutable_cpu()->set_cpu_delay_total(t->cpu_run_virtual_total);
    acct.mutable_cpu()->set_cpu_delay_average(
        average_ms((double) t->cpu_delay_total, t->cpu_count));

    acct.mutable_mem()->set_coremem(t->coremem);
    acct.mutable_mem()->set_virtmem(t->virtmem);
    acct.mutable_mem()->set_hiwater_rss(t->hiwater_rss);
    acct.mutable_mem()->set_hiwater_vm(t->hiwater_vm);

    acct.mutable_ctx()->set_nvcsw(t->nvcsw);
    acct.mutable_ctx()->set_nivcsw(t->nivcsw);

    acct.mutable_io()->set_blkio_count(t->blkio_count);
    acct.mutable_io()->set_blkio_delay_total(t->blkio_delay_total);
    acct.mutable_io()->set_blkio_delay_average(average_ms(t->blkio_delay_total, t->blkio_count));

    acct.mutable_swp()->set_swapin_count(t->swapin_count);
    acct.mutable_swp()->set_swapin_delay_total(t->swapin_delay_total);
    acct.mutable_swp()->set_swapin_delay_average(
        average_ms(t->swapin_delay_total, t->swapin_count));

    acct.mutable_reclaim()->set_freepages_count(t->freepages_count);
    acct.mutable_reclaim()->set_freepages_delay_total(t->freepages_delay_total);
    acct.mutable_reclaim()->set_freepages_delay_average(
        average_ms(t->freepages_delay_total, t->freepages_count));

    acct.mutable_thrashing()->set_thrashing_count(t->thrashing_count);
    acct.mutable_thrashing()->set_thrashing_delay_total(t->thrashing_delay_total);
    acct.mutable_thrashing()->set_thrashing_delay_average(
        average_ms(t->thrashing_delay_total, t->thrashing_count));

    data.mutable_payload()->PackFrom(acct);
    TaskMonitor()->getNetServer()->sendData(data);
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
        processDelayAcct(stats);
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
    long msgBufferSize, txBufferSize, rxBufferSize;
    int err = NLE_SUCCESS;

    try {
        msgBufferSize = std::stol(m_options->getFor(Options::Key::MsgBufferSize));
        rxBufferSize = std::stol(m_options->getFor(Options::Key::TxBufferSize));
        rxBufferSize = std::stol(m_options->getFor(Options::Key::RxBufferSize));
    } catch (...) {
        logWarn() << "Invalid buffer size in config. Use defaults";
        msgBufferSize = 1048576;
        txBufferSize = 1048576;
        rxBufferSize = 1048576;
    }

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

    // We need larger buffers to handle data for all entries
    if (nl_socket_set_buffer_size(m_nlSock, rxBufferSize, txBufferSize) < 0) {
        throw std::runtime_error("Fail to set socket buffer size");
    }

    if (nl_socket_set_msg_buf_size(m_nlSock, msgBufferSize) < 0) {
        throw std::runtime_error("Fail to set socket msg buffer size");
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
                if ((err != -NLE_AGAIN) && (err != -NLE_BUSY) && (err != NLE_OBJ_NOTFOUND)) {
                    logError() << "Error receiving message: " << nl_geterror(err);
                    return true;
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
