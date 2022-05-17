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

#include "ContextEntry.h"
#include "Helpers.h"

namespace tkm::monitor
{

ContextEntry::ContextEntry(uint64_t id, const std::string &name)
{
  m_info.set_ctx_id(id);
  m_info.set_ctx_name(name);
}

void ContextEntry::resetData()
{
  m_info.set_total_cpu_time(0);
  m_info.set_total_cpu_percent(0);
  m_info.set_total_mem_vmrss(0);
}

} // namespace tkm::monitor
