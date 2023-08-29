/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     IUDPSink Class
 * @details   UDP Server Interface
 *-
 */

#pragma once

#include <string>
#include <cstdint>

#include <taskmonitor/taskmonitor.h>

namespace tkm::monitor
{

class IUDPSink
{
public:
  virtual bool active(void) = 0;
  virtual bool send(const tkm::msg::monitor::Data &data) = 0;
};

} // namespace tkm::monitor
