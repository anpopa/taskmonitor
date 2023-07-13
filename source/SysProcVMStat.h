/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcVMStat Class
 * @details   Collect and report information from /proc/meminfo
 *-
 */

#pragma once

#include <taskmonitor/taskmonitor.h>

#include "ICollector.h"
#include "IDataSource.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"

using namespace bswi::event;

namespace tkm::monitor
{
class SysProcVMStat : public IDataSource, public std::enable_shared_from_this<SysProcVMStat>
{
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit SysProcVMStat(const std::shared_ptr<Options> options);
  virtual ~SysProcVMStat() = default;

public:
  SysProcVMStat(SysProcVMStat const &) = delete;
  void operator=(SysProcVMStat const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcVMStat> { return shared_from_this(); }
  auto getProcVMStat() -> tkm::msg::monitor::SysProcVMStat & { return m_data; }
  auto pushRequest(SysProcVMStat::Request &request) -> int;
  void setEventSource(bool enabled = true);
  bool update(void) final;

private:
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  tkm::msg::monitor::SysProcVMStat m_data;
};

} // namespace tkm::monitor
