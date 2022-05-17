/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ContextEntry Class
 * @details   Represent per container statistics
 *-
 */

#pragma once

#include "Monitor.pb.h"
#include <cstdint>
#include <memory>
#include <string>

namespace tkm::monitor
{

class ContextEntry : public std::enable_shared_from_this<ContextEntry>
{
public:
  explicit ContextEntry(uint64_t id, const std::string &name);
  ~ContextEntry() = default;

public:
  ContextEntry(ContextEntry const &) = delete;
  void operator=(ContextEntry const &) = delete;

public:
  auto getShared(void) -> std::shared_ptr<ContextEntry> { return shared_from_this(); }
  auto getInfo(void) -> tkm::msg::monitor::ContextInfo & { return m_info; }
  void setInfo(tkm::msg::monitor::ContextInfo &info) { m_info.CopyFrom(info); }
  auto getContextId(void) -> uint64_t { return m_info.ctx_id(); }
  void resetData(void);

private:
  tkm::msg::monitor::ContextInfo m_info;
};

} // namespace tkm::monitor
