/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcMemInfo Class
 * @details   Collect and report information from /proc/meminfo
 *-
 */

#pragma once

#include <memory>
#include <time.h>
#include <unistd.h>
#include <TaskMonitor.h>

#include "ICollector.h"
#include "IDataSource.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"

using namespace bswi::event;

namespace tkm::monitor
{
class SysProcMemInfo : public IDataSource, public std::enable_shared_from_this<SysProcMemInfo>
{
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit SysProcMemInfo(const std::shared_ptr<Options> options);
  virtual ~SysProcMemInfo() = default;

public:
  SysProcMemInfo(SysProcMemInfo const &) = delete;
  void operator=(SysProcMemInfo const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcMemInfo> { return shared_from_this(); }
  auto getProcMemInfo() -> tkm::msg::monitor::SysProcMemInfo & { return m_memInfo; }
  auto pushRequest(SysProcMemInfo::Request &request) -> int;
  void enableEvents();
  bool update(void) final;

private:
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  tkm::msg::monitor::SysProcMemInfo m_memInfo;
};

} // namespace tkm::monitor
