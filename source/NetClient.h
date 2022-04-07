/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     NetClient Class
 * @details   Network client implementation
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
  
class NetClient : public IClient, public std::enable_shared_from_this<NetClient>
{
public:
  explicit NetClient(int fd);
  ~NetClient();

  void enableEvents();
  auto getShared() -> std::shared_ptr<NetClient> { return shared_from_this(); }

public:
  NetClient(NetClient const &) = delete;
  void operator=(NetClient const &) = delete;
};

} // namespace tkm::monitor
