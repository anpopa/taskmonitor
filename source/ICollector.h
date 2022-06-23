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

#include "EnvelopeReader.h"
#include "EnvelopeWriter.h"

#include <fcntl.h>
#include <memory>
#include <mutex>
#include <unistd.h>

#include "Collector.pb.h"
#include "Monitor.pb.h"

#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ICollector : public Pollable
{
public:
  explicit ICollector(const std::string &name, int fd)
  : Pollable(name, fd)
  , m_reader(std::make_unique<EnvelopeReader>(fd))
  , m_writer(std::make_unique<EnvelopeWriter>(fd))
  {
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

public:
  ICollector(ICollector const &) = delete;
  void operator=(ICollector const &) = delete;

  [[nodiscard]] int getFD() const { return m_fd; }

public:
  tkm::msg::collector::Descriptor descriptor{};
  std::string id{};

private:
  std::unique_ptr<EnvelopeReader> m_reader = nullptr;
  std::unique_ptr<EnvelopeWriter> m_writer = nullptr;
};

} // namespace tkm::monitor
