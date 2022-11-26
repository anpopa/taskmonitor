/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Client Class
 * @details   Dummy client
 *-
 */

#pragma once

#include "Options.h"
#include <string>
#include <taskmonitor/taskmonitor.h>

#include "../../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{

class Client : public Pollable, public std::enable_shared_from_this<Client>
{
public:
  explicit Client(int fd);
  ~Client();

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

  auto getShared() -> std::shared_ptr<Client> { return shared_from_this(); }
  auto getLastEnvelope() -> const tkm::msg::Envelope & { return m_envelope; }
  void setEventSource(bool enabled = true);
  auto getFD(void) -> int { return m_fd; }

public:
  Client(Client const &) = delete;
  void operator=(Client const &) = delete;

private:
  std::unique_ptr<tkm::EnvelopeReader> m_reader = nullptr;
  std::unique_ptr<tkm::EnvelopeWriter> m_writer = nullptr;
  tkm::msg::Envelope m_envelope{};
};

} // namespace tkm::monitor
