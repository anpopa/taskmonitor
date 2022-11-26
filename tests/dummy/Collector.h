/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Collector Class
 * @details   Dummy collector
 *-
 */

#pragma once

#include <string>

#include "ICollector.h"
#include "Options.h"

#include <taskmonitor/taskmonitor.h>

namespace tkm::monitor
{

class Collector : public ICollector, public std::enable_shared_from_this<Collector>
{
public:
  explicit Collector(int fd);
  ~Collector();

  auto getShared() -> std::shared_ptr<Collector> { return shared_from_this(); }
  auto getLastEnvelope() -> const tkm::msg::Envelope & { return m_envelope; }
  void setEventSource(bool enabled = true);

public:
  Collector(Collector const &) = delete;
  void operator=(Collector const &) = delete;

private:
  tkm::msg::Envelope m_envelope{};
};

} // namespace tkm::monitor
