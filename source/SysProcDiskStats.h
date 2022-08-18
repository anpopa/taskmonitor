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

#include <taskmonitor.h>

#include "ICollector.h"
#include "IDataSource.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{
struct DiskStat : public std::enable_shared_from_this<DiskStat> {
public:
  explicit DiskStat(const std::string &name, uint32_t major, uint32_t minor)
  {
    m_data.set_major(major);
    m_data.set_minor(minor);
    m_data.set_name(name);
  };
  ~DiskStat() = default;

public:
  DiskStat(DiskStat const &) = delete;
  void operator=(DiskStat const &) = delete;
  auto getData(void) -> tkm::msg::monitor::DiskStatEntry & { return m_data; }

private:
  tkm::msg::monitor::DiskStatEntry m_data;
};

class SysProcDiskStats : public IDataSource, public std::enable_shared_from_this<SysProcDiskStats>
{
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit SysProcDiskStats(const std::shared_ptr<Options> options);
  virtual ~SysProcDiskStats() = default;

public:
  SysProcDiskStats(SysProcDiskStats const &) = delete;
  void operator=(SysProcDiskStats const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcDiskStats> { return shared_from_this(); }
  auto getDiskStatList() -> bswi::util::SafeList<std::shared_ptr<DiskStat>> & { return m_disks; }
  auto pushRequest(SysProcDiskStats::Request &request) -> int;
  void enableEvents();
  bool update(void) final;

private:
  bool requestHandler(const Request &request);

private:
  bswi::util::SafeList<std::shared_ptr<DiskStat>> m_disks{"DiskStatList"};
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  tkm::msg::monitor::SysProcDiskStats m_diskStats;
};

} // namespace tkm::monitor
