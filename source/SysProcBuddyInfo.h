/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcBuddyInfo Class
 * @details   Collect and report information from /proc/buddyinfo
 *-
 */

#pragma once

#include <taskmonitor/taskmonitor.h>

#include "ICollector.h"
#include "IDataSource.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{

struct BuddyInfo : public std::enable_shared_from_this<BuddyInfo> {
public:
  explicit BuddyInfo(const std::string &name, const std::string &zone)
  {
    m_data.set_name(name);
    m_data.set_zone(zone);
    m_hash = tkm::jnkHsh(std::string(name + zone).c_str());
  };
  ~BuddyInfo() = default;

public:
  BuddyInfo(BuddyInfo const &) = delete;
  void operator=(BuddyInfo const &) = delete;

  auto getHash(void) -> uint64_t { return m_hash; }
  auto getData(void) -> tkm::msg::monitor::BuddyInfo & { return m_data; }

private:
  tkm::msg::monitor::BuddyInfo m_data;
  uint64_t m_hash;
};

class SysProcBuddyInfo : public IDataSource, public std::enable_shared_from_this<SysProcBuddyInfo>
{
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit SysProcBuddyInfo(const std::shared_ptr<Options> options);
  virtual ~SysProcBuddyInfo() = default;

public:
  SysProcBuddyInfo(SysProcBuddyInfo const &) = delete;
  void operator=(SysProcBuddyInfo const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcBuddyInfo> { return shared_from_this(); }
  auto getBuddyInfoList() -> bswi::util::SafeList<std::shared_ptr<BuddyInfo>> & { return m_nodes; }
  auto pushRequest(SysProcBuddyInfo::Request &request) -> int;
  void setEventSource(bool enabled = true);
  bool update(void) final;

private:
  bool requestHandler(const Request &request);

private:
  bswi::util::SafeList<std::shared_ptr<BuddyInfo>> m_nodes{"BuddyInfoList"};
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
};

} // namespace tkm::monitor
