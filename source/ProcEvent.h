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
#include <sys/socket.h>
#include <taskmonitor.h>

#include "ICollector.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ProcEvent : public Pollable, public std::enable_shared_from_this<ProcEvent>
{
public:
  enum class Action { CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit ProcEvent(const std::shared_ptr<Options> options);
  ~ProcEvent(void);

public:
  ProcEvent(ProcEvent const &) = delete;
  void operator=(ProcEvent const &) = delete;

public:
  auto getShared(void) -> std::shared_ptr<ProcEvent> { return shared_from_this(); }
  auto getProcEventData(void) -> tkm::msg::monitor::ProcEvent & { return m_eventData; }
  auto pushRequest(ProcEvent::Request &request) -> int;
  void enableEvents(void);

private:
  void startMonitoring(void);
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<AsyncQueue<ProcEvent::Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  tkm::msg::monitor::ProcEvent m_eventData{};
  struct sockaddr_nl m_addr = {};
  int m_sockFd = -1;
};

} // namespace tkm::monitor
