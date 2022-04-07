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

#include "Options.h"
#include "Server.pb.h"

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/SafeList.h"
#include "../bswinfra/source/Timer.h"

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
  auto getDataSome(void) -> tkm::msg::server::PSIData & { return m_dataSome; }
  auto getDataFull(void) -> tkm::msg::server::PSIData & { return m_dataFull; }

private:
  tkm::msg::server::PSIData m_dataSome;
  tkm::msg::server::PSIData m_dataFull;
  std::string m_name;
};

class SysProcPressure : public std::enable_shared_from_this<SysProcPressure>
{
public:
  explicit SysProcPressure(std::shared_ptr<Options> &options);
  ~SysProcPressure() = default;

public:
  SysProcPressure(SysProcPressure const &) = delete;
  void operator=(SysProcPressure const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcPressure> { return shared_from_this(); }
  void enableEvents();

  void startMonitoring(void);
  void disable(void);

private:
  bool processOnTick(void);

private:
  bswi::util::SafeList<std::shared_ptr<PressureStat>> m_entries{"StatPressureList"};
  std::shared_ptr<Options> m_options = nullptr;
  std::shared_ptr<Timer> m_timer = nullptr;
  size_t m_usecInterval = 0;
};

} // namespace tkm::monitor