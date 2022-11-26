/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Reader Class
 * @details   Dummy reader
 *-
 */

#pragma once

#include "Options.h"
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <taskmonitor/taskmonitor.h>

#include "../../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class Reader : public Pollable, public std::enable_shared_from_this<Reader>
{
public:
  enum class Type { INET, UNIX };

  explicit Reader(const std::shared_ptr<Options> options, Reader::Type type);
  ~Reader();

  auto getShared() -> std::shared_ptr<Reader> { return shared_from_this(); }
  void setEventSource(bool enabled = true);
  auto getFD(void) -> int { return m_fd; }

  auto connect() -> int;
  bool requestData(tkm::msg::collector::Request_Type type);

  auto getSessionInfo() -> const tkm::msg::monitor::SessionInfo & { return m_sessionInfo; };
  auto getLastProcAcct() -> const tkm::msg::monitor::ProcAcct & { return m_procAcct; };
  auto getLastProcInfo() -> const tkm::msg::monitor::ProcInfo & { return m_procInfo; };
  auto getLastProcEvent() -> const tkm::msg::monitor::ProcEvent & { return m_procEvent; };
  auto getLastCtxInfo() -> const tkm::msg::monitor::ContextInfo & { return m_ctxInfo; };
  auto getLastSysProcStat() -> const tkm::msg::monitor::SysProcStat & { return m_sysProcStat; };
  auto getLastSysProcMemInfo() -> const tkm::msg::monitor::SysProcMemInfo &
  {
    return m_sysProcMemInfo;
  };
  auto getLastSysProcDiskStats() -> const tkm::msg::monitor::SysProcDiskStats &
  {
    return m_sysProcDiskStats;
  };
  auto getLastSysProcPressure() -> const tkm::msg::monitor::SysProcPressure &
  {
    return m_sysProcPressure;
  };
  auto getLastSysProcBuddyInfo() -> const tkm::msg::monitor::SysProcBuddyInfo &
  {
    return m_sysProcBuddyInfo;
  };
  auto getLastSysProcWireless() -> const tkm::msg::monitor::SysProcWireless &
  {
    return m_sysProcWireless;
  };

  auto getProcAcctCount() -> size_t { return m_procAcctCount; }
  auto getProcInfoCount() -> size_t { return m_procInfoCount; }
  auto getProcEventCount() -> size_t { return m_procEventCount; }
  auto getCtxInfoCount() -> size_t { return m_ctxInfoCount; }
  auto getSysProcStatCount() -> size_t { return m_sysProcStatCount; }
  auto getSysProcMemInfoCount() -> size_t { return m_sysProcMemInfoCount; }
  auto getSysProcDiskStatsCount() -> size_t { return m_sysProcDiskStatsCount; }
  auto getSysProcPressureCount() -> size_t { return m_sysProcPressureCount; }
  auto getSysProcBuddyInfoCount() -> size_t { return m_sysProcBuddyInfoCount; }
  auto getSysProcWirelessCount() -> size_t { return m_sysProcWirelessCount; }

public:
  Reader(Reader const &) = delete;
  void operator=(Reader const &) = delete;

private:
  auto readEnvelope(tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status
  {
    return m_reader->next(envelope);
  }

  bool writeEnvelope(const tkm::msg::Envelope &envelope)
  {
    if (m_writer->send(envelope) == IAsyncEnvelope::Status::Ok) {
      return m_writer->flush();
    }
    return true;
  }

  auto connectINET() -> int;
  auto connectUNIX() -> int;

private:
  tkm::msg::monitor::SessionInfo m_sessionInfo{};
  tkm::msg::monitor::ProcAcct m_procAcct{};
  tkm::msg::monitor::ProcInfo m_procInfo{};
  tkm::msg::monitor::ProcEvent m_procEvent{};
  tkm::msg::monitor::ContextInfo m_ctxInfo{};
  tkm::msg::monitor::SysProcStat m_sysProcStat{};
  tkm::msg::monitor::SysProcMemInfo m_sysProcMemInfo{};
  tkm::msg::monitor::SysProcDiskStats m_sysProcDiskStats{};
  tkm::msg::monitor::SysProcPressure m_sysProcPressure{};
  tkm::msg::monitor::SysProcBuddyInfo m_sysProcBuddyInfo{};
  tkm::msg::monitor::SysProcWireless m_sysProcWireless{};

  size_t m_procAcctCount = 0;
  size_t m_procInfoCount = 0;
  size_t m_procEventCount = 0;
  size_t m_ctxInfoCount = 0;
  size_t m_sysProcStatCount = 0;
  size_t m_sysProcMemInfoCount = 0;
  size_t m_sysProcDiskStatsCount = 0;
  size_t m_sysProcPressureCount = 0;
  size_t m_sysProcBuddyInfoCount = 0;
  size_t m_sysProcWirelessCount = 0;

private:
  std::unique_ptr<tkm::EnvelopeReader> m_reader = nullptr;
  std::unique_ptr<tkm::EnvelopeWriter> m_writer = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  Reader::Type m_type;
  struct sockaddr_in m_INETAddr {
  };
  struct sockaddr_un m_UNIXAddr {
  };
  int m_sockFd = -1;
};

} // namespace tkm::monitor
