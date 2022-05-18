/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcDiskStats Class
 * @details   Collect and report information from /proc/diskstats
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
class SysProcDiskStats : public std::enable_shared_from_this<SysProcDiskStats>
{
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit SysProcDiskStats(const std::shared_ptr<Options> options);
  ~SysProcDiskStats() = default;

public:
  SysProcDiskStats(SysProcDiskStats const &) = delete;
  void operator=(SysProcDiskStats const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcDiskStats> { return shared_from_this(); }
  auto getProcDiskStats() -> tkm::msg::monitor::SysProcDiskStats & { return m_diskStats; }
  auto pushRequest(SysProcDiskStats::Request &request) -> int;
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
  tkm::msg::monitor::SysProcDiskStats m_diskStats;
  uint64_t m_updateInterval = 1000000;
  bool m_updatePending = false;
};

} // namespace tkm::monitor
