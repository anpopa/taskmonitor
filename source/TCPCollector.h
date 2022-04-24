/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     TCPCollector Class
 * @details   Network TCP client implementation
 *-
 */

#pragma once

#include <string>

#include "ICollector.h"
#include "Options.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::monitor
{

class TCPCollector : public ICollector, public std::enable_shared_from_this<TCPCollector>
{
public:
  explicit TCPCollector(int fd);
  ~TCPCollector();

  auto getShared() -> std::shared_ptr<TCPCollector> { return shared_from_this(); }
  void enableEvents();

public:
  TCPCollector(TCPCollector const &) = delete;
  void operator=(TCPCollector const &) = delete;
};

} // namespace tkm::monitor
