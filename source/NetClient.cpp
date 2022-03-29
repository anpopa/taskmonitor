/*-
 * Copyright (c) 2020 Alin Popa
 * All rights reserved.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
 */

#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "Helpers.h"
#include "NetClient.h"

#include "Client.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

static auto doCreateSession(const shared_ptr<NetClient> &client, tkm::msg::client::Request &rq)
    -> bool;
static auto doStreamState(const shared_ptr<NetClient> &client, tkm::msg::client::Request &rq)
    -> bool;

NetClient::NetClient(int clientFd)
: IClient("NetClient", clientFd)
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

    setFinalize([this]() { logInfo() << "Ended connection with client: " << getFD(); });
}

void NetClient::enableEvents()
{
    TaskMonitor()->addEventSource(getShared());
}

NetClient::~NetClient()
{
    logDebug() << "NetClient " << getFD() << " destructed";
    if (getFD() > 0) {
        ::close(getFD());
    }
}

static auto doCreateSession(const shared_ptr<NetClient> &client, tkm::msg::client::Request &rq)
    -> bool
{
    tkm::msg::Envelope envelope;
    tkm::msg::server::Message message;
    tkm::msg::server::SessionInfo sessionInfo;
    std::string idContent(client->descriptor.id());

    idContent += client->getFD();

    client->id = std::to_string(tkm::jnkHsh(idContent.c_str()));
    sessionInfo.set_id(client->id);

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

static auto doStreamState(const shared_ptr<NetClient> &client, tkm::msg::client::Request &rq)
    -> bool
{
    tkm::msg::client::StreamState streamState;
    rq.data().UnpackTo(&streamState);

    logInfo() << "Received request for set stream state: " << streamState.state();
    client->setStreamEnabled(streamState.state());

    return true;
}

} // namespace tkm::monitor
