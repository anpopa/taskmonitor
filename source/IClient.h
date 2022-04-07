/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     IClient Class
 * @details   Client Interface
 *-
 */

#pragma once

#include "EnvelopeReader.h"
#include "EnvelopeWriter.h"

#include <fcntl.h>
#include <memory>
#include <mutex>
#include <unistd.h>

#include "Client.pb.h"

#include "../bswinfra/source/Pollable.h"

using namespace bswi::event;

namespace tkm::monitor
{
  
class IClient : public Pollable
{
public:
  explicit IClient(const std::string &name, int fd)
  : Pollable(name)
  , m_clientFd(fd)
  {
    m_reader = std::make_unique<EnvelopeReader>(fd);
    m_writer = std::make_unique<EnvelopeWriter>(fd);
  }

  ~IClient() { disconnect(); }

  void disconnect()
  {
    if (m_clientFd > 0) {
      ::close(m_clientFd);
      m_clientFd = -1;
    }
  }

  auto readEnvelope(tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status
  {
    return m_reader->next(envelope);
  }
  auto writeEnvelope(const tkm::msg::Envelope &envelope) -> bool
  {
    if (m_writer->send(envelope) == IAsyncEnvelope::Status::Ok) {
      return m_writer->flush();
    }
    return true;
  }

  void setStreamEnabled(bool state) { m_streamEnabled = state; }
  bool getStreamEnabled(void) { return m_streamEnabled; }

public:
  IClient(IClient const &) = delete;
  void operator=(IClient const &) = delete;

  [[nodiscard]] int getFD() const { return m_clientFd; }

public:
  tkm::msg::client::Descriptor descriptor{};
  std::string id{};

private:
  bool m_streamEnabled = false;
  std::unique_ptr<EnvelopeReader> m_reader = nullptr;
  std::unique_ptr<EnvelopeWriter> m_writer = nullptr;

protected:
  int m_clientFd = -1;
};

} // namespace tkm::monitor
