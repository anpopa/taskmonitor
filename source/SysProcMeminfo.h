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
  explicit SysProcMeminfo(const std::shared_ptr<Options> options);
  ~SysProcMeminfo() = default;

public:
  SysProcMeminfo(SysProcMeminfo const &) = delete;
  void operator=(SysProcMeminfo const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcMeminfo> { return shared_from_this(); }
  auto getProcMemInfo() -> tkm::msg::monitor::SysProcMeminfo & { return m_memInfo; }
  auto pushRequest(SysProcMeminfo::Request &request) -> int;
  void enableEvents();

  void setUpdateInterval(uint64_t interval)
  {
    if (interval > 0) {
      m_updateInterval = interval;
    }
  }
  bool update(void);
  bool getUpdatePending(void) { return m_updatePending; }
  void setUpdatePending(bool state) { m_updatePending = state; }

private:
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  tkm::msg::monitor::SysProcMeminfo m_memInfo;
  uint64_t m_updateInterval = 1000000;
  bool m_updatePending = false;
};

} // namespace tkm::monitor
