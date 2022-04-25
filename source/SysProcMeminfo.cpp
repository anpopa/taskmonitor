/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcMeminfos Class
 * @details   Collect and report information from /proc/meminfo
 *-
 */

#include "SysProcMeminfo.h"
#include "Application.h"
#include "Monitor.pb.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcMeminfo> &mgr,
                          const SysProcMeminfo::Request &request);
static bool doCollectAndSend(const std::shared_ptr<SysProcMeminfo> &mgr,
                             const SysProcMeminfo::Request &request);

SysProcMeminfo::SysProcMeminfo(std::shared_ptr<Options> &options)
: m_options(options)
{
  try {
    m_usecInterval = std::stol(m_options->getFor(Options::Key::MemPollInterval));
  } catch (...) {
    throw std::runtime_error("Fail process MemPollInterval");
  }

  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcMeminfoQueue", [this](const Request &request) { return requestHandler(request); });
  m_timer = std::make_shared<Timer>("SysProcMeminfoTimer", [this]() {
    SysProcMeminfo::Request request = {.action = SysProcMeminfo::Action::UpdateStats};
    return requestHandler(request);
  });
}

auto SysProcMeminfo::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcMeminfo::enableEvents()
{
  // Module queue
  App()->addEventSource(m_queue);

  // Update timer
  m_timer->start(m_usecInterval, true);
  App()->addEventSource(m_timer);
}

auto SysProcMeminfo::requestHandler(const Request &request) -> bool
{
  switch (request.action) {
  case SysProcMeminfo::Action::UpdateStats:
    return doUpdateStats(getShared(), request);
  case SysProcMeminfo::Action::CollectAndSend:
    return doCollectAndSend(getShared(), request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doUpdateStats(const std::shared_ptr<SysProcMeminfo> &mgr,
                          const SysProcMeminfo::Request &request)
{
  std::ifstream memInfoStream{"/proc/meminfo"};

  typedef enum _LineData {
    Unset,
    MemTotal,
    MemFree,
    MemAvailable,
    MemCached,
    SwapTotal,
    SwapFree,
    SwapCached
  } LineData;

  if (!memInfoStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/meminfo file");
  }

  std::string line;
  while (std::getline(memInfoStream, line)) {
    LineData lineData = LineData::Unset;
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    if (line.find("MemTotal") != std::string::npos) {
      lineData = LineData::MemTotal;
    } else if (line.find("MemFree") != std::string::npos) {
      lineData = LineData::MemFree;
    } else if (line.find("MemAvailable") != std::string::npos) {
      lineData = LineData::MemAvailable;
    } else if (line.find("MemCached") != std::string::npos) {
      lineData = LineData::MemCached;
    } else if (line.find("SwapTotal") != std::string::npos) {
      lineData = LineData::SwapTotal;
    } else if (line.find("SwapFree") != std::string::npos) {
      lineData = LineData::SwapFree;
    } else if (line.find("SwapCached") != std::string::npos) {
      lineData = LineData::SwapCached;
    }

    if (lineData == LineData::Unset)
      continue;

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
    case LineData::SwapTotal:
      mgr->getProcMemInfo().set_swap_total(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SwapFree:
      mgr->getProcMemInfo().set_swap_free(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SwapCached:
      mgr->getProcMemInfo().set_swap_cached(std::stoul(tokens[1].c_str()));
      break;
    default:
      break;
    }
  }

  if (mgr->getProcMemInfo().mem_total() > 0) {
    uint32_t memPercent =
        (mgr->getProcMemInfo().mem_available() * 100) / mgr->getProcMemInfo().mem_total();
    mgr->getProcMemInfo().set_mem_percent(memPercent);
  }
  if (mgr->getProcMemInfo().swap_total() > 0) {
    uint32_t swapPercent =
        (mgr->getProcMemInfo().swap_free() * 100) / mgr->getProcMemInfo().swap_total();
    mgr->getProcMemInfo().set_swap_percent(swapPercent);
  }

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcMeminfo> &mgr,
                             const SysProcMeminfo::Request &request)
{
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_SysProcMeminfo);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(currentTime.tv_sec);
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(currentTime.tv_sec);

  data.mutable_payload()->PackFrom(mgr->getProcMemInfo());

  request.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
