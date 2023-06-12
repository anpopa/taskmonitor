/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcMemInfo Class
 * @details   Collect and report information from /proc/meminfo
 *-
 */

#include "SysProcMemInfo.h"
#include "Application.h"

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcMemInfo> mgr);
static bool doCollectAndSend(const std::shared_ptr<SysProcMemInfo> mgr,
                             const SysProcMemInfo::Request &request);

SysProcMemInfo::SysProcMemInfo(const std::shared_ptr<Options> options)
: m_options(options)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcMemInfoQueue", [this](const Request &request) { return requestHandler(request); });
}

auto SysProcMemInfo::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcMemInfo::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(m_queue);
  } else {
    App()->remEventSource(m_queue);
  }
}

bool SysProcMemInfo::update()
{
  if (getUpdatePending()) {
    return true;
  }

  SysProcMemInfo::Request request = {.action = SysProcMemInfo::Action::UpdateStats,
                                     .collector = nullptr};
  bool status = pushRequest(request);

  if (status) {
    setUpdatePending(true);
  }

  return status;
}

auto SysProcMemInfo::requestHandler(const Request &request) -> bool
{
  bool status = false;

  switch (request.action) {
  case SysProcMemInfo::Action::UpdateStats:
    status = doUpdateStats(getShared());
    setUpdatePending(false);
    break;
  case SysProcMemInfo::Action::CollectAndSend:
    status = doCollectAndSend(getShared(), request);
    break;
  default:
    logError() << "Unknown action request";
    break;
  }

  return status;
}

static bool doUpdateStats(const std::shared_ptr<SysProcMemInfo> mgr)
{
  std::ifstream memInfoStream{"/proc/meminfo"};

  typedef enum _LineData {
    Unknown,
    MemTotal,
    MemFree,
    MemAvailable,
    MemCached,
    Active,
    Inactive,
    Slab,
    KReclaimable,
    SReclaimable,
    SUnreclaim,
    KernelStack,
    SwapTotal,
    SwapFree,
    SwapCached,
    CmaTotal,
    CmaFree
  } LineData;

  if (!memInfoStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/meminfo file");
  }

  std::string line;
  while (std::getline(memInfoStream, line)) {
    LineData lineData = LineData::Unknown;
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    if (line.rfind("MemTotal:", 0) != std::string::npos) {
      lineData = LineData::MemTotal;
    } else if (line.rfind("MemFree:", 0) != std::string::npos) {
      lineData = LineData::MemFree;
    } else if (line.rfind("MemAvailable:", 0) != std::string::npos) {
      lineData = LineData::MemAvailable;
    } else if (line.rfind("Cached:", 0) != std::string::npos) {
      lineData = LineData::MemCached;
    } else if (line.rfind("Active:", 0) != std::string::npos) {
      lineData = LineData::Active;
    } else if (line.rfind("Inactive:", 0) != std::string::npos) {
      lineData = LineData::Inactive;
    } else if (line.rfind("Slab:", 0) != std::string::npos) {
      lineData = LineData::Slab;
    } else if (line.rfind("KReclaimable:", 0) != std::string::npos) {
      lineData = LineData::KReclaimable;
    } else if (line.rfind("SReclaimable:", 0) != std::string::npos) {
      lineData = LineData::SReclaimable;
    } else if (line.rfind("SUnreclaim:", 0) != std::string::npos) {
      lineData = LineData::SUnreclaim;
    } else if (line.rfind("KernelStack:", 0) != std::string::npos) {
      lineData = LineData::KernelStack;
    } else if (line.rfind("SwapTotal:", 0) != std::string::npos) {
      lineData = LineData::SwapTotal;
    } else if (line.rfind("SwapFree:", 0) != std::string::npos) {
      lineData = LineData::SwapFree;
    } else if (line.rfind("SwapCached:", 0) != std::string::npos) {
      lineData = LineData::SwapCached;
    } else if (line.rfind("CmaTotal:", 0) != std::string::npos) {
      lineData = LineData::CmaTotal;
    } else if (line.rfind("CmaFree:", 0) != std::string::npos) {
      lineData = LineData::CmaFree;
    }

    if (lineData == LineData::Unknown) {
      continue;
    }

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 2) {
      logError() << "Proc meminfo file parse error";
      return false;
    }

    switch (lineData) {
    case LineData::MemTotal:
      mgr->getProcMemInfo().set_mem_total(std::stoul(tokens[1].c_str()));
      break;
    case LineData::MemFree:
      mgr->getProcMemInfo().set_mem_free(std::stoul(tokens[1].c_str()));
      break;
    case LineData::MemAvailable:
      mgr->getProcMemInfo().set_mem_available(std::stoul(tokens[1].c_str()));
      break;
    case LineData::MemCached:
      mgr->getProcMemInfo().set_mem_cached(std::stoul(tokens[1].c_str()));
      break;
    case LineData::Active:
      mgr->getProcMemInfo().set_active(std::stoul(tokens[1].c_str()));
      break;
    case LineData::Inactive:
      mgr->getProcMemInfo().set_inactive(std::stoul(tokens[1].c_str()));
      break;
    case LineData::Slab:
      mgr->getProcMemInfo().set_slab(std::stoul(tokens[1].c_str()));
      break;
    case LineData::KReclaimable:
      mgr->getProcMemInfo().set_kreclaimable(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SReclaimable:
      mgr->getProcMemInfo().set_sreclaimable(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SUnreclaim:
      mgr->getProcMemInfo().set_sunreclaim(std::stoul(tokens[1].c_str()));
      break;
    case LineData::KernelStack:
      mgr->getProcMemInfo().set_kernel_stack(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SwapTotal:
      mgr->getProcMemInfo().set_swap_total(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SwapFree:
      mgr->getProcMemInfo().set_swap_free(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SwapCached:
      mgr->getProcMemInfo().set_swap_cached(std::stoul(tokens[1].c_str()));
      break;
    case LineData::CmaTotal:
      mgr->getProcMemInfo().set_cma_total(std::stoul(tokens[1].c_str()));
      break;
    case LineData::CmaFree:
      mgr->getProcMemInfo().set_cma_free(std::stoul(tokens[1].c_str()));
      break;
    default:
      break;
    }
  }

  if (mgr->getProcMemInfo().mem_total() > 0) {
    uint64_t memPercent =
        (mgr->getProcMemInfo().mem_available() * 100) / mgr->getProcMemInfo().mem_total();
    mgr->getProcMemInfo().set_mem_percent(static_cast<uint32_t>(memPercent));
  }
  if (mgr->getProcMemInfo().swap_total() > 0) {
    uint64_t swapPercent =
        (mgr->getProcMemInfo().swap_free() * 100) / mgr->getProcMemInfo().swap_total();
    mgr->getProcMemInfo().set_swap_percent(static_cast<uint32_t>(swapPercent));
  }

#ifdef WITH_STARTUP_DATA
  if (App()->getStartupData() != nullptr) {
    if (!App()->getStartupData()->expired()) {
      App()->getStartupData()->addMemData(mgr->getProcMemInfo());
    }
  }
#endif

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcMemInfo> mgr,
                             const SysProcMemInfo::Request &request)
{
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_SysProcMemInfo);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(static_cast<uint64_t>(currentTime.tv_sec));
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(static_cast<uint64_t>(currentTime.tv_sec));

  data.mutable_payload()->PackFrom(mgr->getProcMemInfo());
  request.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
