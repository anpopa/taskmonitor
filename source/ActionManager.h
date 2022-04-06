/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ActionManager Class
 * @details   Application action manager
 *-
 */

#pragma once

#include <map>
#include <string>

#include "NetLinkProc.h"
#include "NetLinkStats.h"
#include "Options.h"

#include "../bswinfra/source/AsyncQueue.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ActionManager : public std::enable_shared_from_this<ActionManager>
{
public:
    enum class Action { RegisterEvents };

    typedef struct Request {
        Action action;
        std::map<std::string, std::string> args;
    } Request;

public:
    explicit ActionManager(std::shared_ptr<Options> &options,
                           std::shared_ptr<NetLinkStats> &nlStats,
                           std::shared_ptr<NetLinkProc> &nlProc);

public:
    auto getShared() -> std::shared_ptr<ActionManager> { return shared_from_this(); }
    void enableEvents();
    auto pushRequest(Request &request) -> int;
    auto getNetLinkStats() -> std::shared_ptr<NetLinkStats> & { return m_nlStats; }
    auto getNetLinkProc() -> std::shared_ptr<NetLinkProc> & { return m_nlProc; }

private:
    auto requestHandler(const Request &request) -> bool;

private:
    std::shared_ptr<Options> m_options = nullptr;
    std::shared_ptr<NetLinkStats> m_nlStats = nullptr;
    std::shared_ptr<NetLinkProc> m_nlProc = nullptr;
    std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
};

} // namespace tkm::monitor
