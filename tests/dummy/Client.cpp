/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Client Class
 * @details   Dummy client implementation
 *-
 */

#include "Client.h"
#include "../tests/dummy/Application.h"
#include "Logger.h"

namespace tkm::monitor
{

Client::Client(int fd)
: Pollable("DummyClient", fd)
, m_reader(std::make_unique<tkm::EnvelopeReader>(fd))
, m_writer(std::make_unique<tkm::EnvelopeWriter>(fd))
{
  bswi::event::Pollable::lateSetup(
      [this]() {
        auto status = true;

        do {
          tkm::msg::Envelope envelope;

          // Read next message
          auto readStatus = readEnvelope(envelope);
          if (readStatus == IAsyncEnvelope::Status::Again) {
            return true;
          } else if (readStatus == IAsyncEnvelope::Status::Error) {
            logDebug() << "Collector read error";
            return false;
          } else if (readStatus == IAsyncEnvelope::Status::EndOfFile) {
            logDebug() << "Collector read end of file";
            return false;
          }

          m_envelope.CopyFrom(envelope);
        } while (status);

        return status;
      },
      fd,
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  setFinalize([this]() { logInfo() << "Ended connection with collector: " << getFD(); });
}

void Client::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(getShared());
  } else {
    App()->remEventSource(getShared());
  }
}

Client::~Client()
{
  logDebug() << "Client " << getFD() << " destructed";
  if (getFD() > 0) {
    ::close(getFD());
  }
}

} // namespace tkm::monitor
