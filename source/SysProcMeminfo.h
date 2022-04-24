/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcMeminfo Class
 * @details   Collect and report information from /proc/meminfo
 *-
 */

#pragma once

#include <memory>
#include <time.h>
#include <unistd.h>

#include "ICollector.h"
#include "Monitor.pb.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{
class SysProcMeminfo : public std::enable_shared_from_this<SysProcMeminfo>
{
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit SysProcMeminfo(std::shared_ptr<Options> &options);
  ~SysProcMeminfo() = default;

public:
  SysProcMeminfo(SysProcMeminfo const &) = delete;
  void operator=(SysProcMeminfo const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcMeminfo> { return shared_from_this(); }
  auto getProcMemInfo() -> tkm::msg::monitor::SysProcMeminfo & { return m_memInfo; }
  auto pushRequest(SysProcMeminfo::Request &request) -> int;
  void enableEvents();

private:
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  tkm::msg::monitor::SysProcMeminfo m_memInfo;
  std::shared_ptr<Timer> m_timer = nullptr;
  uint64_t m_usecInterval = 0;
};

} // namespace tkm::monitor
