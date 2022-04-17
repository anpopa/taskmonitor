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

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace tkm::monitor
{

void SysProcMeminfo::printStats(void)
{
  logInfo() << "SysProcMeminfo[MEM] "
            << "MemTotal=" << m_memInfo.mem_total() << " "
            << "MemFree=" << m_memInfo.mem_free() << " "
            << "MemAvailable=" << m_memInfo.mem_available() << " "
            << "MemCached=" << m_memInfo.mem_cached() << " "
            << "MemAvailablePercent=" << m_memInfo.mem_percent() << "% "
            << "SwapTotal=" << m_memInfo.swap_total() << " "
            << "SwapFree=" << m_memInfo.swap_free() << " "
            << "SwapCached=" << m_memInfo.swap_cached() << " "
            << "SwapFreePercent=" << m_memInfo.swap_percent() << "%";
}

SysProcMeminfo::SysProcMeminfo(std::shared_ptr<Options> &options)
: m_options(options)
{
  try {
    m_usecInterval = std::stol(m_options->getFor(Options::Key::MemPollInterval));
  } catch (...) {
    throw std::runtime_error("Fail process MemPollInterval");
  }

  m_file = std::make_unique<std::ifstream>("/proc/meminfo");
  if (!m_file->is_open()) {
    throw std::runtime_error("Fail to open meminfo file");
  }

  if (options->getFor(Options::Key::SysMemPrintToLog) == "false") {
    m_printToLog = false;
  }

  m_timer = std::make_shared<Timer>("SysProcMeminfo", [this]() { return processOnTick(); });
}

void SysProcMeminfo::startMonitoring(void)
{
  m_timer->start(m_usecInterval, true);
  TaskMonitor()->addEventSource(m_timer);
}

void SysProcMeminfo::disable(void)
{
  m_timer->stop();
  TaskMonitor()->remEventSource(m_timer);
}

bool SysProcMeminfo::processOnTick(void)
{
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

  tkm::msg::server::Data data;
  std::string line;

  data.set_what(tkm::msg::server::Data_What_SysProcMeminfo);
  data.set_timestamp(time(NULL));

  m_file->seekg(0, std::ios::beg);
  while (std::getline(*m_file, line)) {
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
      m_memInfo.set_mem_total(std::stoul(tokens[1].c_str()));
      break;
    case LineData::MemFree:
      m_memInfo.set_mem_free(std::stoul(tokens[1].c_str()));
      break;
    case LineData::MemAvailable:
      m_memInfo.set_mem_available(std::stoul(tokens[1].c_str()));
      break;
    case LineData::MemCached:
      m_memInfo.set_mem_cached(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SwapTotal:
      m_memInfo.set_swap_total(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SwapFree:
      m_memInfo.set_swap_free(std::stoul(tokens[1].c_str()));
      break;
    case LineData::SwapCached:
      m_memInfo.set_swap_cached(std::stoul(tokens[1].c_str()));
      break;
    default:
      break;
    }
  }

  if (m_memInfo.mem_total() > 0) {
    uint32_t memPercent = (m_memInfo.mem_available() * 100) / m_memInfo.mem_total();
    m_memInfo.set_mem_percent(memPercent);
  }
  if (m_memInfo.swap_total() > 0) {
    uint32_t swapPercent = (m_memInfo.swap_free() * 100) / m_memInfo.swap_total();
    m_memInfo.set_swap_percent(swapPercent);
  }

  if (m_printToLog) {
    printStats();
  }

  data.mutable_payload()->PackFrom(m_memInfo);
  TaskMonitor()->getNetServer()->sendData(data);

  return true;
}

} // namespace tkm::monitor
