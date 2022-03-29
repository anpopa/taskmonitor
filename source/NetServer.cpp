/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     NetServer Class
 * @details   Server listening to UDP NetClient connections
 *-
 */

#include <errno.h>
#include <netdb.h>
#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "EnvelopeReader.h"
#include "Helpers.h"
#include "NetClient.h"
#include "NetServer.h"

#include "Client.pb.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

NetServer::NetServer()
: Pollable("NetServer")
{
    if ((m_sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("Fail to create NetServer socket");
    }

    int enable = 1;
    setsockopt(m_sockFd,
               SOL_SOCKET,
               (SO_REUSEPORT | SO_REUSEADDR | SO_DEBUG),
               (char *) &enable,
               sizeof(enable));

    lateSetup(
        [this]() {
            int clientFd = accept(m_sockFd, (struct sockaddr *) nullptr, nullptr);

            if (clientFd < 0) {
                logWarn() << "Fail to accept on NetServer socket";
                return false;
            }

            // The client has 3 seconds to send the descriptor or will be disconnected
            struct timeval tv;
            tv.tv_sec = 3;
            tv.tv_usec = 0;
            setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));

            tkm::msg::client::Descriptor descriptor {};
            if (!readClientDescriptor(clientFd, descriptor)) {
                logWarn() << "Client " << clientFd << " read descriptor failed";
                close(clientFd);
                return true; // this is a client issue, process next client
            }

            logInfo() << "New Client with FD: " << clientFd;
            std::shared_ptr<NetClient> client = std::make_shared<NetClient>(clientFd);

            m_clients.append(client);
            m_clients.commit();

            client->descriptor = descriptor;
            client->enableEvents();

            return true;
        },
        m_sockFd,
        bswi::event::IPollable::Events::Level,
        bswi::event::IEventSource::Priority::Normal);
}

void NetServer::enableEvents()
{
    TaskMonitor()->addEventSource(getShared());
}

NetServer::~NetServer()
{
    static_cast<void>(invalidate());
    m_clients.foreach ([this](const std::shared_ptr<IClient> &entry) { m_clients.remove(entry); });
    m_clients.commit();
}

void NetServer::sendData(const tkm::msg::server::Data &data)
{
    tkm::msg::Envelope envelope;
    tkm::msg::server::Message message;

    message.set_type(tkm::msg::server::Message_Type_Data);
    message.mutable_payload()->PackFrom(data);
    envelope.mutable_mesg()->PackFrom(message);
    envelope.set_target(tkm::msg::Envelope_Recipient_Client);
    envelope.set_origin(tkm::msg::Envelope_Recipient_Server);

    m_clients.foreach ([this, &envelope](const std::shared_ptr<IClient> &entry) {
        if (entry->getStreamEnabled()) {
            entry->writeEnvelope(envelope);
        }
    });
}

void NetServer::notifyClientTerminated(int id)
{
    m_clients.foreach ([this, id](const std::shared_ptr<IClient> &entry) {
        if (entry->getFD() == id) {
            logDebug() << "Found entry to remove with pid " << id;
            m_clients.remove(entry);
        }
    });
    m_clients.commit();
}

void NetServer::bindAndListen()
{
    if (m_bound) {
        logWarn() << "NetServer already listening";
        return;
    }

    // Enable event source
    enableEvents();

    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;

    if (TaskMonitor()->getOptions()->getFor(Options::Key::NetServerAddress) != "any") {
        string serverAddress = TaskMonitor()->getOptions()->getFor(Options::Key::NetServerAddress);
        struct hostent *server = gethostbyname(serverAddress.c_str());
        bcopy(server->h_addr, (char *) &m_addr.sin_addr.s_addr, (size_t) server->h_length);
    }

    auto port = std::stoi(tkmDefaults.getFor(Defaults::Default::NetServerPort));
    try {
        port = std::stoi(TaskMonitor()->getOptions()->getFor(Options::Key::NetServerPort));
    } catch (const std::exception &e) {
        logWarn() << "Cannot convert port number from config: " << e.what();
    }
    m_addr.sin_port = htons(port);

    if (bind(m_sockFd, (struct sockaddr *) &m_addr, sizeof(struct sockaddr_in)) != -1) {
        if (listen(m_sockFd, 100) == -1) {
            logError() << "NetServer listening failed on port: " << port
                       << ". Error: " << strerror(errno);
            throw std::runtime_error("NetServer listen failed");
        }
        logInfo() << "NetServer listening on port: " << port;
    } else {
        logError() << "NetServer bind failed on port: " << port << ". Error: " << strerror(errno);
        throw std::runtime_error("NetServer bind failed");
    }

    m_bound = true;
}

void NetServer::invalidate()
{
    if (m_sockFd > 0) {
        ::close(m_sockFd);
    }
}

} // namespace tkm::monitor
