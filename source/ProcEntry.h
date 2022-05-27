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
  ~ProcEntry() = default;

public:
  ProcEntry(ProcEntry const &) = delete;
  void operator=(ProcEntry const &) = delete;

public:
  auto getShared(void) -> std::shared_ptr<ProcEntry> { return shared_from_this(); }
  bool update(const std::string &sourceName);
  bool update(void);

  auto getAcct(void) -> tkm::msg::monitor::ProcAcct & { return m_acct; }
  void setAcct(tkm::msg::monitor::ProcAcct &acct) { m_acct.CopyFrom(acct); }

  auto getInfo(void) -> tkm::msg::monitor::ProcInfo & { return m_info; }
  void setInfo(tkm::msg::monitor::ProcInfo &info) { m_info.CopyFrom(info); }

  auto getName(void) -> std::string & { return m_name; }
  void setName(const std::string &name)
  {
    m_name = name;
    m_info.set_comm(name);
  }
  auto getPid(void) -> int { return m_pid; }
  auto getContextId(void) -> uint64_t { return m_info.ctx_id(); }

  bool getUpdateProcAcctPending(void) { return m_updateProcAcctPending; }
  void setUpdateProcAcctPending(bool state) { m_updateProcAcctPending = state; }

private:
  void initInfoData(void);
  bool updateInfoData(void);
  bool updateProcAcct(void);
  bool updateProcInfo(void);

private:
  tkm::msg::monitor::ProcAcct m_acct;
  tkm::msg::monitor::ProcInfo m_info;
  bool m_updateProcAcctPending = false;
  std::string m_name{};
  int m_pid = 0;
};

} // namespace tkm::monitor
