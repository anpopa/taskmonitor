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

#include "IDataSource.h"
#include "Monitor.pb.h"
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "../bswinfra/source/Logger.h"

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

  auto getAcct(void) -> tkm::msg::monitor::ProcAcct & { return m_acct; }
  void setAcct(tkm::msg::monitor::ProcAcct &acct) { m_acct.CopyFrom(acct); }

  auto getData(void) -> tkm::msg::monitor::ProcEntry & { return m_proc; }
  void setData(tkm::msg::monitor::ProcEntry &proc) { m_proc.CopyFrom(proc); }

  auto getName(void) -> std::string & { return m_name; }
  void setName(const std::string &name)
  {
    m_name = name;
    m_proc.set_comm(name);
  }
  auto getPid(void) -> int { return m_pid; }
  auto getContextId(void) -> uint64_t { return m_proc.ctx_id(); }

  bool getUpdateProcAcctPending(void) { return m_updateProcAcctPending; }
  void setUpdateProcAcctPending(bool state) { m_updateProcAcctPending = state; }

private:
  void initInfoData(void);
  bool updateInfoData(void);
  bool updateProcAcct(void);
  bool updateProcInfo(void);

private:
  std::chrono::time_point<std::chrono::steady_clock> m_lastUpdateTime{};
  tkm::msg::monitor::ProcAcct m_acct;
  tkm::msg::monitor::ProcEntry m_proc;
  bool m_updateProcAcctPending = false;
  std::string m_name{};
  int m_pid = 0;
};

} // namespace tkm::monitor
