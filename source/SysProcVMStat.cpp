/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcVMStat Class
 * @details   Collect and report information from /proc/meminfo
 *-
 */

#include "SysProcVMStat.h"
#include "Application.h"

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcVMStat> mgr);
static bool doCollectAndSend(const std::shared_ptr<SysProcVMStat> mgr,
                             const SysProcVMStat::Request &request);

SysProcVMStat::SysProcVMStat(const std::shared_ptr<Options> options)
: m_options(options)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcVMStatQueue", [this](const Request &request) { return requestHandler(request); });
}

auto SysProcVMStat::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcVMStat::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(m_queue);
  } else {
    App()->remEventSource(m_queue);
  }
}

bool SysProcVMStat::update()
{
  if (getUpdatePending()) {
    return true;
  }

  SysProcVMStat::Request request = {.action = SysProcVMStat::Action::UpdateStats,
                                    .collector = nullptr};
  bool status = pushRequest(request);

  if (status) {
    setUpdatePending(true);
  }

  return status;
}

auto SysProcVMStat::requestHandler(const Request &request) -> bool
{
  bool status = false;

  switch (request.action) {
  case SysProcVMStat::Action::UpdateStats:
    status = doUpdateStats(getShared());
    setUpdatePending(false);
    break;
  case SysProcVMStat::Action::CollectAndSend:
    status = doCollectAndSend(getShared(), request);
    break;
  default:
    logError() << "Unknown action request";
    break;
  }

  return status;
}

static bool doUpdateStats(const std::shared_ptr<SysProcVMStat> mgr)
{
  std::ifstream memInfoStream{"/proc/vmstat"};

  typedef enum _LineData {
    unknown,
    pgpgin,
    pgpgout,
    pswpin,
    pswpout,
    pgmajfault,
    pgreuse,
    pgsteal_kswapd,
    pgsteal_direct,
    pgsteal_khugepaged,
    pgsteal_anon,
    pgsteal_file,
    pgscan_kswapd,
    pgscan_direct,
    pgscan_khugepaged,
    pgscan_direct_throttle,
    pgscan_anon,
    pgscan_file,
    oom_kill,
    compact_stall,
    compact_fail,
    compact_success,
    thp_fault_alloc,
    thp_collapse_alloc,
    thp_collapse_alloc_failed,
    thp_file_alloc,
    thp_file_mapped,
    thp_split_page,
    thp_split_page_failed,
    thp_zero_page_alloc,
    thp_zero_page_alloc_failed,
    thp_swpout,
    thp_swpout_fallback
  } LineData;

  if (!memInfoStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/vmstat file");
  }

  std::string line;
  while (std::getline(memInfoStream, line)) {
    LineData lineData = LineData::unknown;
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    if (line.rfind("pgpgin", 0) != std::string::npos) {
      lineData = LineData::pgpgin;
    } else if (line.rfind("pgpgout ", 0) != std::string::npos) {
      lineData = LineData::pgpgout;
    } else if (line.rfind("pswpin ", 0) != std::string::npos) {
      lineData = LineData::pswpin;
    } else if (line.rfind("pswpout ", 0) != std::string::npos) {
      lineData = LineData::pswpout;
    } else if (line.rfind("pgmajfault ", 0) != std::string::npos) {
      lineData = LineData::pgmajfault;
    } else if (line.rfind("pgreuse ", 0) != std::string::npos) {
      lineData = LineData::pgreuse;
    } else if (line.rfind("pgsteal_kswapd ", 0) != std::string::npos) {
      lineData = LineData::pgsteal_kswapd;
    } else if (line.rfind("pgsteal_direct ", 0) != std::string::npos) {
      lineData = LineData::pgsteal_direct;
    } else if (line.rfind("pgsteal_khugepaged ", 0) != std::string::npos) {
      lineData = LineData::pgsteal_khugepaged;
    } else if (line.rfind("pgsteal_anon ", 0) != std::string::npos) {
      lineData = LineData::pgsteal_anon;
    } else if (line.rfind("pgsteal_file ", 0) != std::string::npos) {
      lineData = LineData::pgsteal_file;
    } else if (line.rfind("pgscan_kswapd ", 0) != std::string::npos) {
      lineData = LineData::pgscan_kswapd;
    } else if (line.rfind("pgscan_direct ", 0) != std::string::npos) {
      lineData = LineData::pgscan_direct;
    } else if (line.rfind("pgscan_khugepaged ", 0) != std::string::npos) {
      lineData = LineData::pgscan_khugepaged;
    } else if (line.rfind("pgscan_direct_throttle ", 0) != std::string::npos) {
      lineData = LineData::pgscan_direct_throttle;
    } else if (line.rfind("pgscan_anon ", 0) != std::string::npos) {
      lineData = LineData::pgscan_anon;
    } else if (line.rfind("pgscan_file ", 0) != std::string::npos) {
      lineData = LineData::pgscan_file;
    } else if (line.rfind("oom_kill ", 0) != std::string::npos) {
      lineData = LineData::oom_kill;
    } else if (line.rfind("compact_stall ", 0) != std::string::npos) {
      lineData = LineData::compact_stall;
    } else if (line.rfind("compact_fail ", 0) != std::string::npos) {
      lineData = LineData::compact_fail;
    } else if (line.rfind("compact_success ", 0) != std::string::npos) {
      lineData = LineData::compact_success;
    } else if (line.rfind("thp_fault_alloc ", 0) != std::string::npos) {
      lineData = LineData::thp_fault_alloc;
    } else if (line.rfind("thp_collapse_alloc ", 0) != std::string::npos) {
      lineData = LineData::thp_collapse_alloc;
    } else if (line.rfind("thp_collapse_alloc_failed ", 0) != std::string::npos) {
      lineData = LineData::thp_collapse_alloc_failed;
    } else if (line.rfind("thp_file_alloc ", 0) != std::string::npos) {
      lineData = LineData::thp_file_alloc;
    } else if (line.rfind("thp_file_mapped ", 0) != std::string::npos) {
      lineData = LineData::thp_file_mapped;
    } else if (line.rfind("thp_split_page ", 0) != std::string::npos) {
      lineData = LineData::thp_split_page;
    } else if (line.rfind("thp_split_page_failed ", 0) != std::string::npos) {
      lineData = LineData::thp_split_page_failed;
    } else if (line.rfind("thp_zero_page_alloc ", 0) != std::string::npos) {
      lineData = LineData::thp_zero_page_alloc;
    } else if (line.rfind("thp_zero_page_alloc_failed ", 0) != std::string::npos) {
      lineData = LineData::thp_zero_page_alloc_failed;
    } else if (line.rfind("thp_swpout ", 0) != std::string::npos) {
      lineData = LineData::thp_swpout;
    } else if (line.rfind("thp_swpout_fallback ", 0) != std::string::npos) {
      lineData = LineData::thp_swpout_fallback;
    }

    if (lineData == LineData::unknown) {
      continue;
    }

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 2) {
      logError() << "Proc vmstat file parse error";
      return false;
    }

    switch (lineData) {
    case LineData::pgpgin:
      mgr->getProcVMStat().set_pgpgin(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgpgout:
      mgr->getProcVMStat().set_pgpgout(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pswpin:
      mgr->getProcVMStat().set_pswpin(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pswpout:
      mgr->getProcVMStat().set_pswpout(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgmajfault:
      mgr->getProcVMStat().set_pgmajfault(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgreuse:
      mgr->getProcVMStat().set_pgreuse(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgsteal_kswapd:
      mgr->getProcVMStat().set_pgsteal_kswapd(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgsteal_direct:
      mgr->getProcVMStat().set_pgsteal_direct(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgsteal_khugepaged:
      mgr->getProcVMStat().set_pgsteal_khugepaged(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgsteal_anon:
      mgr->getProcVMStat().set_pgsteal_anon(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgsteal_file:
      mgr->getProcVMStat().set_pgsteal_file(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgscan_kswapd:
      mgr->getProcVMStat().set_pgscan_kswapd(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgscan_direct:
      mgr->getProcVMStat().set_pgscan_direct(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgscan_khugepaged:
      mgr->getProcVMStat().set_pgscan_khugepaged(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgscan_direct_throttle:
      mgr->getProcVMStat().set_pgscan_direct_throttle(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgscan_anon:
      mgr->getProcVMStat().set_pgscan_anon(std::stoul(tokens[1].c_str()));
      break;
    case LineData::pgscan_file:
      mgr->getProcVMStat().set_pgscan_file(std::stoul(tokens[1].c_str()));
      break;
    case LineData::oom_kill:
      mgr->getProcVMStat().set_oom_kill(std::stoul(tokens[1].c_str()));
      break;
    case LineData::compact_stall:
      mgr->getProcVMStat().set_compact_stall(std::stoul(tokens[1].c_str()));
      break;
    case LineData::compact_fail:
      mgr->getProcVMStat().set_compact_fail(std::stoul(tokens[1].c_str()));
      break;
    case LineData::compact_success:
      mgr->getProcVMStat().set_compact_success(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_fault_alloc:
      mgr->getProcVMStat().set_thp_fault_alloc(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_collapse_alloc:
      mgr->getProcVMStat().set_thp_collapse_alloc(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_collapse_alloc_failed:
      mgr->getProcVMStat().set_thp_collapse_alloc_failed(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_file_alloc:
      mgr->getProcVMStat().set_thp_file_alloc(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_file_mapped:
      mgr->getProcVMStat().set_thp_file_mapped(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_split_page:
      mgr->getProcVMStat().set_thp_split_page(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_split_page_failed:
      mgr->getProcVMStat().set_thp_split_page_failed(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_zero_page_alloc:
      mgr->getProcVMStat().set_thp_zero_page_alloc(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_zero_page_alloc_failed:
      mgr->getProcVMStat().set_thp_zero_page_alloc_failed(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_swpout:
      mgr->getProcVMStat().set_thp_swpout(std::stoul(tokens[1].c_str()));
      break;
    case LineData::thp_swpout_fallback:
      mgr->getProcVMStat().set_thp_swpout_fallback(std::stoul(tokens[1].c_str()));
      break;
    default:
      break;
    }
  }

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcVMStat> mgr,
                             const SysProcVMStat::Request &request)
{
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_SysProcVMStat);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(static_cast<uint64_t>(currentTime.tv_sec));
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(static_cast<uint64_t>(currentTime.tv_sec));

  data.mutable_payload()->PackFrom(mgr->getProcVMStat());
  request.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
