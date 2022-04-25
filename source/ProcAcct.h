/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcAcct Class
 * @details   Collect process statistics for each ProcEntry
 *-
 */

#pragma once

#include <netinet/in.h>
#include <netlink/netlink.h>
#include <string>
#include <sys/socket.h>

#include "Options.h"
#include "ProcEntry.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/IApplication.h"
#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ProcAcct : public Pollable, public std::enable_shared_from_this<ProcAcct>
{
public:
  explicit ProcAcct(std::shared_ptr<Options> &options);
  ~ProcAcct();

public:
  ProcAcct(ProcAcct const &) = delete;
  void operator=(ProcAcct const &) = delete;

public:
  auto getShared() -> std::shared_ptr<ProcAcct> { return shared_from_this(); }
  void enableEvents();

  bool requestTaskAcct(int pid);

private:
  std::shared_ptr<Options> m_options = nullptr;
  struct nl_sock *m_nlSock = nullptr;
  int m_nlFamily = 0;
  int m_sockFd = -1;
};

} // namespace tkm::monitor
