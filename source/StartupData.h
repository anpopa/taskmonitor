/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     StartupData Class
 * @details   Collect and report information from /proc/meminfo
 *-
 */

#pragma once

#include <taskmonitor.h>

#include "ICollector.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"

using namespace bswi::event;

namespace tkm::monitor
{

class StartupData : public std::enable_shared_from_this<StartupData>
{

  struct CPUStartupData {
  public:
    explicit CPUStartupData(const tkm::msg::monitor::SysProcStat &data)
    {
      struct timespec currentTime;

      clock_gettime(CLOCK_REALTIME, &currentTime);
      m_systemTime = currentTime.tv_sec;
      clock_gettime(CLOCK_MONOTONIC, &currentTime);
      m_monotonicTime = currentTime.tv_sec;

      m_data.CopyFrom(data);
    };
    ~CPUStartupData() = default;

  public:
    CPUStartupData(CPUStartupData const &) = delete;
    void operator=(CPUStartupData const &) = delete;

    auto getData(void) -> const tkm::msg::monitor::SysProcStat & { return m_data; }
    auto getMonotonicTime(void) -> uint64_t { return m_monotonicTime; };
    auto getSystemTime(void) -> uint64_t { return m_systemTime; };

  private:
    tkm::msg::monitor::SysProcStat m_data;
    uint64_t m_monotonicTime;
    uint64_t m_systemTime;
  };

  struct MEMStartupData {
  public:
    explicit MEMStartupData(const tkm::msg::monitor::SysProcMemInfo &data)
    {
      struct timespec currentTime;

      clock_gettime(CLOCK_REALTIME, &currentTime);
      m_systemTime = currentTime.tv_sec;
      clock_gettime(CLOCK_MONOTONIC, &currentTime);
      m_monotonicTime = currentTime.tv_sec;

      m_data.CopyFrom(data);
    };
    ~MEMStartupData() = default;

  public:
    MEMStartupData(MEMStartupData const &) = delete;
    void operator=(MEMStartupData const &) = delete;

    auto getData(void) -> const tkm::msg::monitor::SysProcMemInfo & { return m_data; }
    auto getMonotonicTime(void) -> uint64_t { return m_monotonicTime; };
    auto getSystemTime(void) -> uint64_t { return m_systemTime; };

  private:
    tkm::msg::monitor::SysProcMemInfo m_data;
    uint64_t m_monotonicTime;
    uint64_t m_systemTime;
  };

  struct PSIStartupData {
  public:
    explicit PSIStartupData(const tkm::msg::monitor::SysProcPressure &data)
    {
      struct timespec currentTime;

      clock_gettime(CLOCK_REALTIME, &currentTime);
      m_systemTime = currentTime.tv_sec;
      clock_gettime(CLOCK_MONOTONIC, &currentTime);
      m_monotonicTime = currentTime.tv_sec;

      m_data.CopyFrom(data);
    };
    ~PSIStartupData() = default;

  public:
    PSIStartupData(PSIStartupData const &) = delete;
    void operator=(PSIStartupData const &) = delete;

    auto getData(void) -> const tkm::msg::monitor::SysProcPressure & { return m_data; }
    auto getMonotonicTime(void) -> uint64_t { return m_monotonicTime; };
    auto getSystemTime(void) -> uint64_t { return m_systemTime; };

  private:
    tkm::msg::monitor::SysProcPressure m_data;
    uint64_t m_monotonicTime;
    uint64_t m_systemTime;
  };

public:
  enum class Action { CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit StartupData(const std::shared_ptr<Options> options);
  virtual ~StartupData() = default;

public:
  StartupData(StartupData const &) = delete;
  void operator=(StartupData const &) = delete;

public:
  auto getShared() -> std::shared_ptr<StartupData> { return shared_from_this(); }

  bool expired() { return m_expired; }
  void dropData();

  auto getCpuList() -> const std::vector<std::shared_ptr<CPUStartupData>> & { return m_cpuList; }
  auto getMemList() -> const std::vector<std::shared_ptr<MEMStartupData>> & { return m_memList; }
  auto getPsiList() -> const std::vector<std::shared_ptr<PSIStartupData>> & { return m_psiList; }
  void addCpuData(const tkm::msg::monitor::SysProcStat &data);
  void addMemData(const tkm::msg::monitor::SysProcMemInfo &data);
  void addPsiData(const tkm::msg::monitor::SysProcPressure &data);

  auto pushRequest(StartupData::Request &request) -> int;
  void enableEvents();

private:
  bool requestHandler(const Request &request);

private:
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::vector<std::shared_ptr<CPUStartupData>> m_cpuList;
  std::vector<std::shared_ptr<MEMStartupData>> m_memList;
  std::vector<std::shared_ptr<PSIStartupData>> m_psiList;
  bool m_expired = false;
};

} // namespace tkm::monitor
