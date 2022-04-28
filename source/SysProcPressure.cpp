/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcPresure Class
 * @details   Collect and report PSI information
 *-
 */

#include "SysProcPressure.h"
#include "Application.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcPressure> mgr,
                          const SysProcPressure::Request &request);
static bool doCollectAndSend(const std::shared_ptr<SysProcPressure> mgr,
                             const SysProcPressure::Request &request);

static auto tokenize(const std::string &str, const char &ch) -> std::vector<std::string>
{
  std::string next;
  std::vector<std::string> result;

  for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
    if (*it == ch) {
      if (!next.empty()) {
        result.push_back(next);
        next.clear();
      }
    } else {
      next += *it;
    }
  }

  if (!next.empty())
    result.push_back(next);

  return result;
}

void PressureStat::updateStats(void)
{
  std::ifstream file("/proc/pressure/" + m_name);

  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      std::vector<std::string> tokens;
      std::stringstream ss(line);
      std::string buf;

      while (ss >> buf) {
        tokens.push_back(buf);
      }

      tkm::msg::monitor::PSIData data;
      for (size_t i = 1; i < tokens.size(); i++) {
        std::vector<std::string> keyVal = tokenize(tokens[i], '=');

        if (keyVal.size() == 2) {
          if (keyVal[0] == "avg10") {
            data.set_avg10(std::stof(keyVal[1]));
          } else if (keyVal[0] == "avg60") {
            data.set_avg60(std::stof(keyVal[1]));
          } else if (keyVal[0] == "avg300") {
            data.set_avg300(std::stof(keyVal[1]));
          } else if (keyVal[0] == "total") {
            data.set_total(std::stoul(keyVal[1]));
          }
        }
      }

      if (tokens[0] == "some") {
        m_dataSome = data;
      } else {
        m_dataFull = data;
      }
    }
  } else {
    logError() << "Cannot open pressure file: "
               << "/proc/pressure/" + m_name;
  }
}

SysProcPressure::SysProcPressure(const std::shared_ptr<Options> options)
: m_options(options)
{
  try {
    m_usecInterval = std::stol(m_options->getFor(Options::Key::PressurePollInterval));
  } catch (...) {
    throw std::runtime_error("Fail process PressurePollInterval");
  }

  if (std::filesystem::exists("/proc/pressure/cpu")) {
    std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("cpu");
    m_entries.append(entry);
  }
  if (std::filesystem::exists("/proc/pressure/memory")) {
    std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("memory");
    m_entries.append(entry);
  }
  if (std::filesystem::exists("/proc/pressure/io")) {
    std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("io");
    m_entries.append(entry);
  }
  m_entries.commit();

  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcPressureQueue", [this](const Request &request) { return requestHandler(request); });
  m_timer = std::make_shared<Timer>("SysProcMeminfoTimer", [this]() {
    SysProcPressure::Request request = {.action = SysProcPressure::Action::UpdateStats};
    return requestHandler(request);
  });
}

auto SysProcPressure::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcPressure::enableEvents()
{
  // Module queue
  App()->addEventSource(m_queue);

  // Update timer
  m_timer->start(m_usecInterval, true);
  App()->addEventSource(m_timer);
}

auto SysProcPressure::requestHandler(const Request &request) -> bool
{
  switch (request.action) {
  case SysProcPressure::Action::UpdateStats:
    return doUpdateStats(getShared(), request);
  case SysProcPressure::Action::CollectAndSend:
    return doCollectAndSend(getShared(), request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

static bool doUpdateStats(const std::shared_ptr<SysProcPressure> mgr,
                          const SysProcPressure::Request &request)
{
  mgr->getProcEntries().foreach ([&mgr](const std::shared_ptr<PressureStat> &entry) {
    entry->updateStats();
    if (entry->getName() == "cpu") {
      mgr->getProcPressure().mutable_cpu_some()->CopyFrom(entry->getDataSome());
      mgr->getProcPressure().mutable_cpu_full()->CopyFrom(entry->getDataFull());
    } else if (entry->getName() == "memory") {
      mgr->getProcPressure().mutable_mem_some()->CopyFrom(entry->getDataSome());
      mgr->getProcPressure().mutable_mem_full()->CopyFrom(entry->getDataFull());
    } else if (entry->getName() == "io") {
      mgr->getProcPressure().mutable_io_some()->CopyFrom(entry->getDataSome());
      mgr->getProcPressure().mutable_io_full()->CopyFrom(entry->getDataFull());
    }
  });

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcPressure> mgr,
                             const SysProcPressure::Request &request)
{
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_SysProcPressure);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(currentTime.tv_sec);
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(currentTime.tv_sec);

  data.mutable_payload()->PackFrom(mgr->getProcPressure());
  request.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
