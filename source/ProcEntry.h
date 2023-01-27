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

#pragma once

#include <chrono>
#include <string>
#include <taskmonitor/taskmonitor.h>

#include "IDataSource.h"

namespace tkm::monitor
{

class ProcEntry : public IDataSource, public std::enable_shared_from_this<ProcEntry>
{
public:
  explicit ProcEntry(int pid, const std::string &name);
  virtual ~ProcEntry() = default;

public:
  ProcEntry(ProcEntry const &) = delete;
  void operator=(ProcEntry const &) = delete;

public:
  auto getShared(void) -> std::shared_ptr<ProcEntry> { return shared_from_this(); }
  bool update(const std::string &sourceName) override;
  bool update(void) override;

#ifdef WITH_PROC_ACCT
  auto getAcct(void) -> tkm::msg::monitor::ProcAcct & { return m_acct; }
  void setAcct(tkm::msg::monitor::ProcAcct &acct) { m_acct.CopyFrom(acct); }
  bool getUpdateProcAcctPending(void) { return m_updateProcAcctPending; }
  void setUpdateProcAcctPending(bool state) { m_updateProcAcctPending = state; }
#endif

  auto getData(void) -> tkm::msg::monitor::ProcInfoEntry & { return m_info; }
  void setData(tkm::msg::monitor::ProcInfoEntry &proc) { m_info.CopyFrom(proc); }

  auto getName(void) -> const std::string & { return m_info.comm(); }
  void setName(const std::string &name) { m_info.set_comm(name); }
  auto getPid(void) -> int { return m_pid; }
  auto getContextId(void) -> uint64_t { return m_info.ctx_id(); }

private:
  void initInfoData(void);
  bool updateInfoData(void);
#ifdef WITH_PROC_ACCT
  bool updateProcAcct(void);
#endif
  bool updateProcInfo(void);

private:
  std::chrono::time_point<std::chrono::steady_clock> m_lastUpdateTime{};
  tkm::msg::monitor::ProcInfoEntry m_info;
#ifdef WITH_PROC_ACCT
  tkm::msg::monitor::ProcAcct m_acct;
  bool m_updateProcAcctPending = false;
#endif
#ifdef WITH_LXC
  size_t m_contextNameResolveCount = 0;
#endif
  int m_pid = 0;
};

} // namespace tkm::monitor
