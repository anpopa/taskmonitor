/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Registry Class
 * @details   ProcEntry registry
 *-
 */

#pragma once

#include <list>
#include <memory>
#include <string>

#include "Options.h"
#include "ProcEntry.h"

#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{

class Registry
{
public:
    explicit Registry(std::shared_ptr<Options> &options)
    : m_options(options)
    {
        long pollInterval = -1;

        try {
            pollInterval = std::stol(m_options->getFor(Options::Key::ProcPollInterval));
        } catch (...) {
            // Discard non pid entries
        }

        if (pollInterval != -1) {
            m_pollInterval = pollInterval;
        }
    };
    ~Registry() = default;

public:
    Registry(Registry const &) = delete;
    void operator=(Registry const &) = delete;

public:
    void initFromProc(void);
    long getPollInterval(void) { return m_pollInterval; }
    void addEntry(int pid);
    void remEntry(int pid);
    auto getEntry(int pid) -> const std::shared_ptr<ProcEntry>;

private:
    bool isBlacklisted(int pid);

private:
    std::shared_ptr<Options> m_options = nullptr;
    bswi::util::SafeList<std::shared_ptr<ProcEntry>> m_list {"RegistryList"};
    long m_pollInterval = 3000000;
};

} // namespace tkm::monitor
