/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcPresure Class
 * @details   Collect and report PSI information
 *-
 */

#pragma once

#include <time.h>
#include <unistd.h>

#include "ICollector.h"
#include "IDataSource.h"
#include "Monitor.pb.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{

struct PressureStat : public std::enable_shared_from_this<PressureStat> {
public:
  explicit PressureStat(const std::string &name)
  : m_name(name){};
  ~PressureStat() = default;

public:
  PressureStat(PressureStat const &) = delete;
  void operator=(PressureStat const &) = delete;

  void updateStats(void);
  auto getName(void) -> const std::string & { return m_name; }
  auto getDataSome(void) -> tkm::msg::monitor::PSIData & { return m_dataSome; }
  auto getDataFull(void) -> tkm::msg::monitor::PSIData & { return m_dataFull; }

private:
  tkm::msg::monitor::PSIData m_dataSome;
  tkm::msg::monitor::PSIData m_dataFull;
  std::string m_name;
};

class SysProcPressure : public IDataSource, public std::enable_shared_from_this<SysProcPressure>
{
public:
public:
  enum class Action { UpdateStats, CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

  explicit SysProcPressure(const std::shared_ptr<Options> options);
  ~SysProcPressure() = default;

public:
  SysProcPressure(SysProcPressure const &) = delete;
  void operator=(SysProcPressure const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcPressure> { return shared_from_this(); }
  auto pushRequest(SysProcPressure::Request &request) -> int;
  auto getProcPressure() -> tkm::msg::monitor::SysProcPressure & { return m_psiData; }
  auto getProcEntries() -> bswi::util::SafeList<std::shared_ptr<PressureStat>> &
  {
    return m_entries;
  }
  void enableEvents();
  bool update(void) final;

private:
  bool requestHandler(const Request &request);

private:
  bswi::util::SafeList<std::shared_ptr<PressureStat>> m_entries{"StatPressureList"};
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  tkm::msg::monitor::SysProcPressure m_psiData;
};

} // namespace tkm::monitor
