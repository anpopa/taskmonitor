/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcEntry Class
 * @details   Represent per process statistics
 *-
 */

#include "ProcEntry.h"
#include "Application.h"
#include "Helpers.h"

namespace tkm::monitor
{

ProcEntry::ProcEntry(int pid, const std::string &name)
: m_name(name)
, m_pid(pid)
{
  initInfoData();
}

bool ProcEntry::updateProcAcct(void)
{
  if (App()->getProcAcctCollectorCounter() == 0) {
    return true;
  }

  if (getUpdateProcAcctPending()) {
    return true;
  }

  setUpdateProcAcctPending(true);
  if (!App()->getProcAcct()->requestTaskAcct(m_pid)) {
    App()->getProcRegistry()->remProcEntry(m_pid);
    return false;
  }

  return true;
}

bool ProcEntry::updateProcInfo(void)
{
  if (getUpdatePending()) {
    return true;
  }

  setUpdatePending(true);
  auto status = updateInfoData();
  setUpdatePending(false);

  if (!status) {
    App()->getProcRegistry()->remProcEntry(m_pid);
    return false;
  }

  return true;
}

void ProcEntry::initInfoData(void)
{
  std::ifstream statStream{"/proc/" + std::to_string(m_pid) + "/stat"};

  if (!statStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/" + std::to_string(m_pid) + "/stat file");
  }

  std::string line;
  if (std::getline(statStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 52) {
      throw std::runtime_error("Parse fail for /proc/" + std::to_string(m_pid) + "stat file");
    }

    auto afterNameOffset = tokens.size() - 52;
    m_info.set_pid(static_cast<uint32_t>(std::stoul(tokens[0])));
    m_info.set_ppid(static_cast<uint32_t>(std::stoul(tokens[3 + afterNameOffset])));

    auto cpuTime =
        std::stoul(tokens[13 + afterNameOffset]) + std::stoul(tokens[14 + afterNameOffset]);
    m_info.set_cpu_time(cpuTime);
  }

  m_info.set_comm(m_name);
  m_info.set_ctx_id(tkm::getContextId(m_pid));
  m_info.set_ctx_name(tkm::getContextName(App()->getOptions()->getFor(Options::Key::ContainersPath),
                                          m_info.ctx_id()));
}

bool ProcEntry::updateInfoData(void)
{
  std::ifstream statStream{"/proc/" + std::to_string(m_pid) + "/stat"};

  if (!statStream.is_open()) {
    logWarn() << "Fail to open stat file for pid " << m_pid;
    return false;
  }

  std::string line;
  if (std::getline(statStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 52) {
      logError() << "Fail to parse state file for pid " << m_pid;
      return false;
    }

    auto afterNameOffset = tokens.size() - 52;
    uint64_t oldCPUTime = m_info.cpu_time();
    uint64_t newCPUTime =
        std::stoul(tokens[13 + afterNameOffset]) + std::stoul(tokens[14 + afterNameOffset]);

    m_info.set_cpu_time(newCPUTime);

    auto timeNow = std::chrono::steady_clock::now();
    using USec = std::chrono::microseconds;

    if (m_lastUpdateTime.time_since_epoch().count() == 0) {
      m_lastUpdateTime = timeNow;
      m_info.set_cpu_percent(0);
    } else {
      auto durationUs = std::chrono::duration_cast<USec>(timeNow - m_lastUpdateTime).count();
      m_lastUpdateTime = timeNow;
      m_info.set_cpu_percent(((newCPUTime - oldCPUTime) * 1000000) / durationUs);
    }

    m_info.set_mem_vmrss(std::stoul(tokens[23]) * ::sysconf(_SC_PAGESIZE) / 1024);
  }

  return true;
}

bool ProcEntry::update(const std::string &sourceName)
{
  if (tkmDefaults.valFor(Defaults::Val::ProcAcct) == sourceName) {
    return updateProcAcct();
  }
  return updateProcInfo();
}

bool ProcEntry::update(void)
{
  auto status = updateProcInfo();
  if (status) {
    status = updateProcAcct();
  }
  return status;
}

} // namespace tkm::monitor
