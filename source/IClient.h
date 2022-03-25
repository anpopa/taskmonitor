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

#include <fcntl.h>
#include <memory>
#include <mutex>
#include <unistd.h>

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
    }

    ~IClient() { disconnect(); }

    void disconnect()
    {
        if (m_clientFd > 0) {
            ::close(m_clientFd);
            m_clientFd = -1;
        }
    }

    virtual void writePayloadString(const std::string &str) = 0;

public:
    IClient(IClient const &) = delete;
    void operator=(IClient const &) = delete;

    [[nodiscard]] int getFD() const { return m_clientFd; }

private:
    std::string m_ownerName {};

protected:
    int m_clientFd = -1;
};

} // namespace tkm::monitor
