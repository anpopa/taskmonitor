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
#include "JsonWriter.h"
#include "NetLinkStats.h"

using std::shared_ptr;
using std::string;

#define average_ms(t, c) (t / 1000000ULL / (c ? c : 1))

static bool withCPU = false;
static bool withMemory = false;
static bool withContext = false;
static bool withIO = false;
static bool withSwap = false;
static bool withReclaim = false;
static bool withTrashing = false;

static void processDelayAcct(struct taskstats *t)
{
    auto entry = TaskMonitor()->getRegistry()->getEntry(t->ac_pid);

    if (entry == nullptr) {
        logError() << "Stat entry with PID " << t->ac_pid << " not in registry";
        return;
    }

    Json::Value head;
    head["type"] = "stats";
    head["time"] = time(NULL);

    Json::Value common;
    common["ac_comm"] = t->ac_comm;
    common["ac_uid"] = t->ac_uid;
    common["ac_gid"] = t->ac_gid;
    common["ac_pid"] = t->ac_pid;
    common["ac_ppid"] = t->ac_ppid;
    common["ac_utime"] = static_cast<unsigned long>(t->ac_utime);
    common["ac_stime"] = static_cast<unsigned long>(t->ac_stime);
    common["user_cpu_percent"] = entry->getUserCPUPercent(t->ac_utime);
    common["sys_cpu_percent"] = entry->getSystemCPUPercent(t->ac_stime);
    head["common"] = common;

    if (withCPU) {
        Json::Value cpu;
        cpu["cpu_count"] = static_cast<unsigned long>(t->cpu_count);
        cpu["cpu_run_real_total"] = static_cast<unsigned long>(t->cpu_run_real_total);
        cpu["cpu_run_virtual_total"] = static_cast<unsigned long>(t->cpu_run_virtual_total);
        cpu["cpu_delay_total"] = static_cast<unsigned long>(t->cpu_delay_total);
        cpu["cpu_delay_average"] = average_ms((double) t->cpu_delay_total, t->cpu_count);
        head["cpu"] = cpu;
    }

    if (withMemory) {
        Json::Value memory;
        memory["coremem"] = static_cast<unsigned long>(t->coremem);
        memory["virtmem"] = static_cast<unsigned long>(t->virtmem);
        memory["hiwater_rss"] = static_cast<unsigned long>(t->hiwater_rss);
        memory["hiwater_vm"] = static_cast<unsigned long>(t->hiwater_vm);
        head["memory"] = memory;
    }

    if (withContext) {
        Json::Value context;
        context["nvcsw"] = static_cast<unsigned long>(t->nvcsw);
        context["nivcsw"] = static_cast<unsigned long>(t->nivcsw);
        head["context"] = context;
    }

    if (withIO) {
        Json::Value io;
        io["blkio_count"] = static_cast<unsigned long>(t->blkio_count);
        io["blkio_delay_total"] = static_cast<unsigned long>(t->blkio_delay_total);
        io["blkio_delay_average"]
            = static_cast<unsigned long>(average_ms(t->blkio_delay_total, t->blkio_count));
        head["io"] = io;
    }

    if (withSwap) {
        Json::Value swap;
        swap["swapin_count"] = static_cast<unsigned long>(t->swapin_count);
        swap["swapin_delay_total"] = static_cast<unsigned long>(t->swapin_delay_total);
        swap["swapin_delay_average"]
            = static_cast<unsigned long>(average_ms(t->swapin_delay_total, t->swapin_count));
        head["swap"] = swap;
    }

    if (withReclaim) {
        Json::Value reclaim;
        reclaim["freepages_count"] = static_cast<unsigned long>(t->freepages_count);
        reclaim["freepages_delay_total"] = static_cast<unsigned long>(t->freepages_delay_total);
        reclaim["freepages_delay_average"]
            = static_cast<unsigned long>(average_ms(t->freepages_delay_total, t->freepages_count));
        head["reclaim"] = reclaim;
    }

    if (withTrashing) {
        Json::Value trashing;
        trashing["thrashing_count"] = static_cast<unsigned long>(t->thrashing_count);
        trashing["thrashing_delay_total"] = static_cast<unsigned long>(t->thrashing_delay_total);
        trashing["thrashing_delay_average"]
            = static_cast<unsigned long>(average_ms(t->thrashing_delay_total, t->thrashing_count));
        head["trashing"] = trashing;
    }

    writeJsonStream() << head;
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

    if (m_options->getFor(Options::Key::WithCPU) == "true") {
        withCPU = true;
    }

    if (m_options->getFor(Options::Key::WithMemory) == "true") {
        withMemory = true;
    }

    if (m_options->getFor(Options::Key::WithContext) == "true") {
        withContext = true;
    }

    if (m_options->getFor(Options::Key::WithIO) == "true") {
        withIO = true;
    }

    if (m_options->getFor(Options::Key::WithSwap) == "true") {
        withSwap = true;
    }

    if (m_options->getFor(Options::Key::WithReclaim) == "true") {
        withReclaim = true;
    }

    if (m_options->getFor(Options::Key::WithTrashing) == "true") {
        withTrashing = true;
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
