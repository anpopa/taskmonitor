/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     TCPClient Class
 * @details   Network TCP client implementation
 *-
 */

#pragma once

#include <string>

#include "IClient.h"
#include "Options.h"

using namespace bswi::log;
using namespace bswi::event;

namespace tkm::monitor
{

class TCPClient : public IClient, public std::enable_shared_from_this<TCPClient>
{
public:
  explicit TCPClient(int fd);
  ~TCPClient();

  void enableEvents();
  auto getShared() -> std::shared_ptr<TCPClient> { return shared_from_this(); }

public:
  TCPClient(TCPClient const &) = delete;
  void operator=(TCPClient const &) = delete;
};

} // namespace tkm::monitor
