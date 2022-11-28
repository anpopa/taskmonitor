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

#ifdef WITH_PROC_ACCT
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
#endif

bool ProcEntry::updateProcInfo(void)
{
  bool status = true;

  if (getUpdatePending()) {
    return true;
  }
  setUpdatePending(true);

  try {
    updateInfoData();
  } catch (std::exception &e) {
    logError() << "Fail to update info data for PID " << m_pid << ". Exception: " << e.what();
    status = false;
  }

  setUpdatePending(false);
  if (!status) {
    App()->getProcRegistry()->remProcEntry(m_pid);
  }

  return status;
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

  // If need to run as root for context identification
  if (getuid() == 0) {
    m_info.set_ctx_id(tkm::getContextId(m_pid));
    m_info.set_ctx_name(tkm::getContextName(
        App()->getOptions()->getFor(Options::Key::ContainersPath), m_info.ctx_id()));
  } else {
    m_info.set_ctx_id(0);
    m_info.set_ctx_name("generic");
  }
}

void ProcEntry::updateInfoData(void)
{
  std::string line;

  std::ifstream statStream{"/proc/" + std::to_string(m_pid) + "/stat"};
  if (std::getline(statStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 52) {
      throw std::runtime_error("Fail to parse stat file");
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
      m_info.set_cpu_percent(static_cast<uint32_t>(((newCPUTime - oldCPUTime) * 1000000) /
                                                   static_cast<uint64_t>(durationUs)));
    }
  }

  std::ifstream statmStream{"/proc/" + std::to_string(m_pid) + "/statm"};
  if (std::getline(statmStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    // We are only interested in the first 3 values
    if (tokens.size() < 3) {
      throw std::runtime_error("Fail to parse statm file");
    }

    const auto pageSize = static_cast<uint64_t>(::sysconf(_SC_PAGESIZE));

    // Store memory sizes in Kb
    m_info.set_mem_vmsize(std::stoul(tokens[0]) * pageSize / 1024);
    m_info.set_mem_vmrss(std::stoul(tokens[1]) * pageSize / 1024);
    m_info.set_mem_shared(std::stoul(tokens[2]) * pageSize / 1024);
  }
}

bool ProcEntry::update(const std::string &sourceName)
{
  if (tkmDefaults.valFor(Defaults::Val::ProcAcct) == sourceName) {
#ifdef WITH_PROC_ACCT
    return updateProcAcct();
#else
    return false;
#endif
  }
  return updateProcInfo();
}

bool ProcEntry::update(void)
{
  auto status = updateProcInfo();
#ifdef WITH_PROC_ACCT
  if (status) {
    status = updateProcAcct();
  }
#endif
  return status;
}

} // namespace tkm::monitor
