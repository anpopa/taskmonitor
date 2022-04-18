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
class SysProcMeminfo : public std::enable_shared_from_this<SysProcMeminfo>
{
public:
  explicit SysProcMeminfo(std::shared_ptr<Options> &options);
  ~SysProcMeminfo() = default;

public:
  SysProcMeminfo(SysProcMeminfo const &) = delete;
  void operator=(SysProcMeminfo const &) = delete;

public:
  auto getShared() -> std::shared_ptr<SysProcMeminfo> { return shared_from_this(); }
  void enableEvents();

  void startMonitoring(void);
  void disable(void);

private:
  bool processOnTick(void);
  void printStats(void);

private:
  tkm::msg::server::SysProcMeminfo m_memInfo;
  std::shared_ptr<Options> m_options = nullptr;
  std::shared_ptr<Timer> m_timer = nullptr;
  bool m_printToLog = true;
  size_t m_usecInterval = 0;
};

} // namespace tkm::monitor
