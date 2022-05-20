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
#include "ProcAcct.h"
#include <cstdint>

#include <limits.h>
#include <unistd.h>

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
  if (App()->getProcAcctCollectorCounter() <= 0) {
    return true;
  }

  if (getUpdateProcAcctPending()) {
    return true;
  }

  setUpdateProcAcctPending(true);
  if (!App()->getProcAcct()->requestTaskAcct(m_pid)) {
    App()->getRegistry()->remProcEntry(m_pid);
    return false;
  }

  if (!updateInfoData()) {
    App()->getRegistry()->remProcEntry(m_pid);
  }

  return true;
}

bool ProcEntry::updateProcInfo(void)
{
  if (getUpdateProcInfoPending()) {
    return true;
  }

  setUpdateProcInfoPending(true);
  auto status = updateInfoData();
  setUpdateProcInfoPending(false);

  if (!status) {
    App()->getRegistry()->remProcEntry(m_pid);
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

    // Our intervals are in nanoseconds so we muliply by ns in s
    m_info.set_cpu_percent(((newCPUTime - oldCPUTime) * 1000000) / m_updateProcInfoInterval);
    m_info.set_mem_vmrss(std::stoul(tokens[23]) * ::sysconf(_SC_PAGESIZE) / 1024);
  }

  return true;
}

} // namespace tkm::monitor
