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

#include "Monitor.pb.h"
#include <cstdint>
#include <memory>
#include <string>

#include "../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ProcEntry : public std::enable_shared_from_this<ProcEntry>
{
public:
  explicit ProcEntry(int pid, const std::string &name);
  ~ProcEntry() = default;

public:
  ProcEntry(ProcEntry const &) = delete;
  void operator=(ProcEntry const &) = delete;

public:
  auto getShared() -> std::shared_ptr<ProcEntry> { return shared_from_this(); }
  void startMonitoring(unsigned int interval);
  void disable(void);

  auto getAcct() -> const tkm::msg::monitor::ProcAcct & { return m_acct; }
  void setAcct(tkm::msg::monitor::ProcAcct &acct) { m_acct.CopyFrom(acct); }
  auto getName() -> std::string & { return m_name; }
  auto getPid() -> int { return m_pid; }

private:
  std::shared_ptr<Timer> m_timer = nullptr;
  tkm::msg::monitor::ProcAcct m_acct;
  unsigned int m_pollInterval = 0;
  std::string m_name{};
  int m_pid = 0;
};

} // namespace tkm::monitor
