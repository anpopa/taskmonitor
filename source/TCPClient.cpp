/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     TCPClient Class
 * @details   Network TCP client implementation
 *-
 */

#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "Helpers.h"
#include "TCPClient.h"

#include "Client.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

static bool doCreateSession(const shared_ptr<TCPClient> &client, tkm::msg::client::Request &rq);
static bool doStreamState(const shared_ptr<TCPClient> &client, tkm::msg::client::Request &rq);

TCPClient::TCPClient(int clientFd)
: IClient("TCPClient", clientFd)
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
            logDebug() << "Client read error";
            return false;
          } else if (readStatus == IAsyncEnvelope::Status::EndOfFile) {
            logDebug() << "Client read end of file";
            return false;
          }

          // Handle generic client request
          if (envelope.origin() != msg::Envelope_Recipient_Client) {
            ::close(getFD());
            return false;
          }

          tkm::msg::client::Request clientMessage;
          envelope.mesg().UnpackTo(&clientMessage);

          switch (clientMessage.type()) {
          case tkm::msg::client::Request_Type_CreateSession:
            status = doCreateSession(getShared(), clientMessage);
            break;
          case tkm::msg::client::Request_Type_StreamState:
            status = doStreamState(getShared(), clientMessage);
            break;
          default:
            status = false;
            break;
          }

        } while (status);

        return status;
      },
      getFD(),
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  setFinalize([this]() {
    logInfo() << "Ended connection with client: " << getFD();
    App()->getTCPServer()->notifyClientTerminated(getFD());
  });
}

void TCPClient::enableEvents()
{
  App()->addEventSource(getShared());
}

TCPClient::~TCPClient()
{
  logDebug() << "TCPClient " << getFD() << " destructed";
  if (getFD() > 0) {
    ::close(getFD());
  }
}

static bool doCreateSession(const shared_ptr<TCPClient> &client, tkm::msg::client::Request &rq)
{
  tkm::msg::Envelope envelope;
  tkm::msg::server::Message message;
  tkm::msg::server::SessionInfo sessionInfo;
  std::string idContent(client->descriptor.id());

  char randData[64] = {0};
  srandom(time(0));
  snprintf(randData, sizeof(randData), "%0lX", random());
  idContent += randData;

  logInfo() << "Session hash content: " << idContent
            << " jenkinsHash: " << tkm::jnkHsh(idContent.c_str());
  client->id = std::to_string(tkm::jnkHsh(idContent.c_str()));
  sessionInfo.set_id(client->id);
  logInfo() << "Send new sessionID=" << sessionInfo.id();

  // TODO: Don't know how to get LC ID yet
  sessionInfo.set_lifecycleid("na");

  message.set_type(tkm::msg::server::Message::Type::Message_Type_SetSession);
  message.mutable_payload()->PackFrom(sessionInfo);

  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Client);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Server);

  logDebug() << "Send session id: " << sessionInfo.id() << " to client: " << client->getFD();
  return client->writeEnvelope(envelope);
}

static bool doStreamState(const shared_ptr<TCPClient> &client, tkm::msg::client::Request &rq)
{
  tkm::msg::client::StreamState streamState;
  rq.data().UnpackTo(&streamState);

  logInfo() << "Received request for set stream state: " << streamState.state();
  client->setStreamEnabled(streamState.state());

  return true;
}

} // namespace tkm::monitor
