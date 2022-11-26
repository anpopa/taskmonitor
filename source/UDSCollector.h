/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     UDSCollector Class
 * @details   Network UDS client implementation
 *-
 */

#pragma once

#include <string>

#include "ICollector.h"
#include "Options.h"

namespace tkm::monitor
{

class UDSCollector : public ICollector, public std::enable_shared_from_this<UDSCollector>
{
public:
  explicit UDSCollector(int fd);
  ~UDSCollector();

  auto getShared() -> std::shared_ptr<UDSCollector> { return shared_from_this(); }
  void setEventSource(bool enabled = true);

public:
  UDSCollector(UDSCollector const &) = delete;
  void operator=(UDSCollector const &) = delete;
};

} // namespace tkm::monitor
