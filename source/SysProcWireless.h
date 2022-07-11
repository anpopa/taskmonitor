/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcWireless Class
 * @details   Collect and report information from /proc/net/wireless
 *-
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <time.h>
#include <unistd.h>

#include "Helpers.h"
#include "ICollector.h"
#include "IDataSource.h"
#include "Monitor.pb.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{

struct WlanInterface : public std::enable_shared_from_this<WlanInterface> {
public:
  explicit WlanInterface(const std::string &name) { m_data.set_name(name); };
  ~WlanInterface() = default;

public:
  WlanInterface(WlanInterface const &) = delete;
  void operator=(WlanInterface const &) = delete;

  auto getData(void) -> tkm::msg::monitor::WlanInterface & { return m_data; }

private:
  tkm::msg::monitor::WlanInterface m_data;
};

class SysProcWireless : public IDataSource, public std::enable_shared_from_this<SysProcWireless>
{
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit SysProcWireless(const std::shared_ptr<Options> options);
  virtual ~SysProcWireless() = default;

public:
  SysProcWireless(SysProcWireless const &) = delete;
  void operator=(SysProcWireless const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcWireless> { return shared_from_this(); }
  auto getWlanInterfaceList() -> bswi::util::SafeList<std::shared_ptr<WlanInterface>> &
  {
    return m_nodes;
  }
  auto pushRequest(SysProcWireless::Request &request) -> int;
  void enableEvents();
  bool update(void) final;

private:
  bool requestHandler(const Request &request);

private:
  bswi::util::SafeList<std::shared_ptr<WlanInterface>> m_nodes{"WlanInterfaceList"};
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
};

} // namespace tkm::monitor
