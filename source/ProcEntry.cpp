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

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

bool gProcInfoFDCollect = false;

namespace tkm::monitor
{

#ifdef WITH_LXC
constexpr size_t ContextNameMaxRetry = 10;
#endif

ProcEntry::ProcEntry(int pid, const std::string &name)
: m_pid(pid)
{
  setName(name);
  initInfoData();

  if (App()->getOptions() != nullptr) {
    if (App()->getOptions()->getFor(Options::Key::EnableProcFDCount) ==
        tkmDefaults.valFor(Defaults::Val::True)) {
      gProcInfoFDCollect = true;
    }
  }
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

#ifdef WITH_LXC
  if (m_contextNameResolveCount++ < ContextNameMaxRetry) {
    if (m_info.ctx_name() == "unknown") {
      m_info.set_ctx_id(tkm::getContextId(m_pid));
      m_info.set_ctx_name(tkm::getContextName(
          App()->getOptions()->getFor(Options::Key::ContainersPath), m_info.ctx_id()));
    }
  }
#endif

  try {
    status = updateInfoData();
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
  if (!std::getline(statStream, line)) {
    throw std::runtime_error("Fail to read /proc/" + std::to_string(m_pid) + "/stat file");
  }

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

bool ProcEntry::readProcStat(void)
{
  std::ifstream statStream{"/proc/" + std::to_string(m_pid) + "/stat"};
  if (!statStream.is_open()) {
    return false;
  }

  std::string line;
  if (!std::getline(statStream, line)) {
    return false;
  }

  std::vector<std::string> tokens;
  std::string buf;

  std::stringstream ss(line);
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

  return true;
}

bool ProcEntry::readProcSmapsRollup(void)
{
  std::ifstream memStream{"/proc/" + std::to_string(m_pid) + "/smaps_rollup"};
  if (!memStream.is_open()) {
    return false;
  }

  std::vector<std::string> tokens;
  std::string line;
  std::string buf;

  typedef enum _LineMemData { IGN, RSS, PSS } LineMemData;

  size_t complete = 2;
  while (std::getline(memStream, line)) {
    LineMemData lineData = LineMemData::IGN;
    std::stringstream ssm(line);

    if (line.rfind("Rss:", 0) != std::string::npos) {
      lineData = LineMemData::RSS;
    } else if (line.rfind("Pss:", 0) != std::string::npos) {
      lineData = LineMemData::PSS;
    }

    if (lineData == LineMemData::IGN) {
      continue;
    }

    while (ssm >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 3) {
      logError() << "Proc smaps_rollup file parse error";
      return false;
    }

    switch (lineData) {
    case LineMemData::RSS:
      m_info.set_mem_rss(std::stoull(tokens[1]));
      complete--;
      break;
    case LineMemData::PSS:
      m_info.set_mem_pss(std::stoull(tokens[1]));
      complete--;
      break;
    default:
      break;
    }

    if (complete == 0) {
      break;
    }
    tokens.clear();
  }

  return true;
}

bool ProcEntry::countFileDescriptors(void)
{
  auto dirIter = fs::directory_iterator("/proc/" + std::to_string(m_pid) + "/fd");
  auto fdCount = m_info.fd_count();

  try {
    fdCount = std::count_if(begin(dirIter), end(dirIter), [](auto &entry) {
#if __has_include(<filesystem>)
      return entry.is_symlink();
#else
      return fs::is_symlink(entry);
#endif
    });
  } catch (...) {
    return false;
  }

  m_info.set_fd_count(fdCount);

  return true;
}

bool ProcEntry::updateInfoData(void)
{
  if (!readProcStat()) {
    return false;
  }

  if (!readProcSmapsRollup()) {
    return false;
  }

  if (gProcInfoFDCollect) {
    if (!countFileDescriptors()) {
      return false;
    }
  }

  return true;
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
