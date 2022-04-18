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

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace tkm::monitor
{

std::vector<std::string> tokenize(const std::string &str, const char &ch)
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

      tkm::msg::server::PSIData data;
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

SysProcPressure::SysProcPressure(std::shared_ptr<Options> &options)
: m_options(options)
{
  try {
    m_usecInterval = std::stol(m_options->getFor(Options::Key::PressurePollInterval));
  } catch (...) {
    throw std::runtime_error("Fail process pressure PollInterval");
  }

  if (options->getFor(Options::Key::PressureWithCPU) == "true") {
    std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("cpu");
    m_entries.append(entry);
  }
  if (options->getFor(Options::Key::PressureWithMemory) == "true") {
    std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("memory");
    m_entries.append(entry);
  }
  if (options->getFor(Options::Key::PressureWithIO) == "true") {
    std::shared_ptr<PressureStat> entry = std::make_shared<PressureStat>("io");
    m_entries.append(entry);
  }
  m_entries.commit();

  m_timer = std::make_shared<Timer>("SysProcPressure", [this]() { return processOnTick(); });
}

void SysProcPressure::startMonitoring(void)
{
  m_timer->start(m_usecInterval, true);
  App()->addEventSource(m_timer);
}

void SysProcPressure::disable(void)
{
  m_timer->stop();
  App()->remEventSource(m_timer);
}

bool SysProcPressure::processOnTick(void)
{
  tkm::msg::server::Data data;
  tkm::msg::server::SysProcPressure psiEvent;

  data.set_what(tkm::msg::server::Data_What_SysProcPressure);
  data.set_timestamp(time(NULL));

  m_entries.foreach ([this, &psiEvent](const std::shared_ptr<PressureStat> &entry) {
    entry->updateStats();
    if (entry->getName() == "cpu") {
      psiEvent.mutable_cpu_some()->CopyFrom(entry->getDataSome());
      psiEvent.mutable_cpu_full()->CopyFrom(entry->getDataFull());
    } else if (entry->getName() == "memory") {
      psiEvent.mutable_mem_some()->CopyFrom(entry->getDataSome());
      psiEvent.mutable_mem_full()->CopyFrom(entry->getDataFull());
    } else if (entry->getName() == "io") {
      psiEvent.mutable_io_some()->CopyFrom(entry->getDataSome());
      psiEvent.mutable_io_full()->CopyFrom(entry->getDataFull());
    }
  });

  data.mutable_payload()->PackFrom(psiEvent);
  App()->getTCPServer()->sendData(data);

  return true;
}

} // namespace tkm::monitor
