/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     StartupData Class
 * @details   Collect and report information from /proc/meminfo
 *-
 */

#include "StartupData.h"
#include "Application.h"
#include "Logger.h"
#include "Monitor.pb.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../bswinfra/source/Timer.h"

namespace tkm::monitor
{

static bool doCollectAndSend(const std::shared_ptr<StartupData> mgr,
                             const StartupData::Request &request);

StartupData::StartupData(const std::shared_ptr<Options> options)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "StartupDataQueue", [this](const Request &request) { return requestHandler(request); });

  auto expireTimer = std::make_shared<Timer>("StartupDataTimer", []() {
    if (App()->getStartupData() != nullptr) {
      App()->getStartupData()->dropData();
    }
    return false;
  });

  auto timeout = std::stoul(options->getFor(Options::Key::StartupDataCleanupTime));
  logDebug() << "Startup data will expire in " << timeout << " usec";
  expireTimer->start(timeout, false);
  App()->addEventSource(expireTimer);
}

auto StartupData::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void StartupData::enableEvents()
{
  App()->addEventSource(m_queue);
}

void StartupData::dropData()
{
  if (!m_expired) {
    logDebug() << "Startup data cache dropped";
    m_cpuList.clear();
    m_memList.clear();
    m_psiList.clear();
    m_expired = true;
  }
}

void StartupData::addCpuData(const tkm::msg::monitor::SysProcStat &data)
{
  if (!m_expired) {
    auto entry = std::make_shared<CPUStartupData>(data);
    m_cpuList.push_back(entry);
  }
}

void StartupData::addMemData(const tkm::msg::monitor::SysProcMemInfo &data)
{
  if (!m_expired) {
    auto entry = std::make_shared<MEMStartupData>(data);
    m_memList.push_back(entry);
  }
}

void StartupData::addPsiData(const tkm::msg::monitor::SysProcPressure &data)
{
  if (!m_expired) {
    auto entry = std::make_shared<PSIStartupData>(data);
    m_psiList.push_back(entry);
  }
}

auto StartupData::requestHandler(const Request &request) -> bool
{
  bool status = false;

  switch (request.action) {
  case StartupData::Action::CollectAndSend:
    status = doCollectAndSend(getShared(), request);
    break;
  default:
    logError() << "Unknown action request";
    break;
  }

  return status;
}

static bool doCollectAndSend(const std::shared_ptr<StartupData> mgr,
                             const StartupData::Request &request)
{
  if (mgr->expired()) {
    return true;
  }

  for (const auto &cpuData : mgr->getCpuList()) {
    tkm::msg::monitor::Data data;

    data.set_what(tkm::msg::monitor::Data_What_SysProcStat);
    data.set_system_time_sec(cpuData->getSystemTime());
    data.set_monotonic_time_sec(cpuData->getMonotonicTime());
    data.mutable_payload()->PackFrom(cpuData->getData());

    request.collector->sendData(data);
  }

  for (const auto &memData : mgr->getMemList()) {
    tkm::msg::monitor::Data data;

    data.set_what(tkm::msg::monitor::Data_What_SysProcMemInfo);
    data.set_system_time_sec(memData->getSystemTime());
    data.set_monotonic_time_sec(memData->getMonotonicTime());
    data.mutable_payload()->PackFrom(memData->getData());

    request.collector->sendData(data);
  }

  for (const auto &psiData : mgr->getPsiList()) {
    tkm::msg::monitor::Data data;

    data.set_what(tkm::msg::monitor::Data_What_SysProcPressure);
    data.set_system_time_sec(psiData->getSystemTime());
    data.set_monotonic_time_sec(psiData->getMonotonicTime());
    data.mutable_payload()->PackFrom(psiData->getData());

    request.collector->sendData(data);
  }

  return true;
}

} // namespace tkm::monitor
