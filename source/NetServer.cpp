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
#include "NetClient.h"
#include "NetServer.h"

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

            logInfo() << "New NetClient with FD: " << clientFd;
            std::shared_ptr<NetClient> client = std::make_shared<NetClient>(clientFd);

            m_clients.append(client);
            m_clients.commit();

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
    static_cast<void>(stop());
    m_clients.foreach ([this](const std::shared_ptr<IClient> &entry) { m_clients.remove(entry); });
    m_clients.commit();
}

void NetServer::broadcastPayloadString(const std::string &str)
{
    m_clients.foreach (
        [this, &str](const std::shared_ptr<IClient> &entry) { entry->writePayloadString(str); });
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

void NetServer::start()
{
    string serverAddress = TaskMonitor()->getOptions()->getFor(Options::Key::NetServerAddress);
    struct hostent *server = gethostbyname(serverAddress.c_str());

    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    bcopy(server->h_addr, (char *) &m_addr.sin_addr.s_addr, (size_t) server->h_length);

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
}

void NetServer::stop()
{
    if (m_sockFd > 0) {
        ::close(m_sockFd);
    }
}

} // namespace tkm::monitor
