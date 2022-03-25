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
#include "NetClient.h"

using std::shared_ptr;
using std::string;

namespace tkm::monitor
{

NetClient::NetClient(int clientFd)
: IClient("NetClient", clientFd)
{
    bswi::event::Pollable::lateSetup(
        [this]() {
            // If we receive something we disconnect
            return false;
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

void NetClient::writePayloadString(const std::string &str)
{
    auto len = send(getFD(), str.c_str(), str.size(), MSG_NOSIGNAL);
    if (len == -1) {
        logWarn() << "Fail to send data to client id " << getFD();
        TaskMonitor()->getNetServer()->notifyClientTerminated(getFD());
    }
}

} // namespace tkm::monitor
