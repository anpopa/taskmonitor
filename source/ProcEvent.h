/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcEvent Class
 * @details   Monitor system process events using netlink interfaces
 *-
 */

#pragma once

#include <linux/netlink.h>
#include <string>
#include <sys/socket.h>

#include "Options.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/IApplication.h"
#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ProcEvent : public Pollable, public std::enable_shared_from_this<ProcEvent>
{
public:
  explicit ProcEvent(std::shared_ptr<Options> &options);
  ~ProcEvent();

public:
  ProcEvent(ProcEvent const &) = delete;
  void operator=(ProcEvent const &) = delete;

public:
  auto getShared() -> std::shared_ptr<ProcEvent> { return shared_from_this(); }
  void enableEvents();
  auto startProcMonitoring(void) -> int;
  [[nodiscard]] int getFD() const { return m_sockFd; }

private:
  std::shared_ptr<Options> m_options = nullptr;
  struct sockaddr_nl m_addr = {};
  int m_sockFd = -1;
};

} // namespace tkm::monitor
