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

namespace tkm::monitor
{

ContextEntry::ContextEntry(uint64_t id, const std::string &name)
{
  m_data.set_ctx_id(id);
  m_data.set_ctx_name(name);
}

void ContextEntry::resetData()
{
  m_data.set_total_cpu_time(0);
  m_data.set_total_cpu_percent(0);
  m_data.set_total_mem_rss(0);
  m_data.set_total_mem_pss(0);
}

} // namespace tkm::monitor
