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

#include <taskmonitor/taskmonitor.h>

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
  auto getData(void) -> tkm::msg::monitor::ContextInfoEntry & { return m_data; }
  void setData(tkm::msg::monitor::ContextInfoEntry &data) { m_data.CopyFrom(data); }
  auto getContextId(void) -> uint64_t { return m_data.ctx_id(); }
  void resetData(void);

private:
  tkm::msg::monitor::ContextInfoEntry m_data;
};

} // namespace tkm::monitor
