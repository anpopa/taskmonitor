/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ICollector Class
 * @details   Collector Interface
 *-
 */

#pragma once

#include <taskmonitor/taskmonitor.h>

#include "../bswinfra/source/Pollable.h"
#include "../bswinfra/source/Timer.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ICollector : public Pollable
{
public:
  enum class Type { TCP, UDS };

  explicit ICollector(const std::string &name, ICollector::Type type, int fd)
  : Pollable(name, fd)
  , m_type(type)
  , m_reader(std::make_unique<tkm::EnvelopeReader>(fd))
  , m_writer(std::make_unique<tkm::EnvelopeWriter>(fd))
  {
    setLastUpdateTime(std::chrono::steady_clock::now());
  }

  ~ICollector() { disconnect(); }

  void disconnect()
  {
    if (m_fd > 0) {
      ::close(m_fd);
      m_fd = -1;
    }
  }

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

  void sendData(const tkm::msg::monitor::Data &data)
  {
    tkm::msg::Envelope envelope;
    tkm::msg::monitor::Message message;

    message.set_type(tkm::msg::monitor::Message_Type_Data);
    message.mutable_payload()->PackFrom(data);
    envelope.mutable_mesg()->PackFrom(message);
    envelope.set_target(tkm::msg::Envelope_Recipient_Collector);
    envelope.set_origin(tkm::msg::Envelope_Recipient_Monitor);

    writeEnvelope(envelope);
  }

  auto getDescriptor(void) -> tkm::msg::collector::Descriptor & { return m_descriptor; }
  auto getSessionInfo(void) -> tkm::msg::monitor::SessionInfo & { return m_sessionInfo; }
  auto getFD(void) -> int { return m_fd; }
  auto getType(void) -> const ICollector::Type { return m_type; }

  void setLastUpdateTime(std::chrono::time_point<std::chrono::steady_clock> newTime)
  {
    m_lastUpdateTime = newTime;
  }
  auto getLastUpdateTime(void) -> std::chrono::time_point<std::chrono::steady_clock>
  {
    return m_lastUpdateTime;
  }

public:
  ICollector(ICollector const &) = delete;
  void operator=(ICollector const &) = delete;

private:
  std::chrono::time_point<std::chrono::steady_clock> m_lastUpdateTime{};
  std::unique_ptr<tkm::EnvelopeReader> m_reader = nullptr;
  std::unique_ptr<tkm::EnvelopeWriter> m_writer = nullptr;
  tkm::msg::collector::Descriptor m_descriptor{};
  tkm::msg::monitor::SessionInfo m_sessionInfo{};
  ICollector::Type m_type;
};

} // namespace tkm::monitor
