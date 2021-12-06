/*
 * SPDX license identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2021 Alin Popa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * \author Alin Popa <alin.popa@fxdata.ro>
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
            pollInterval = std::stol(m_options->getFor(Options::Key::PollInterval));
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

    void initFromProc(void);
    void addEntry(int pid);
    void remEntry(int pid);

private:
    std::shared_ptr<Options> m_options = nullptr;
    bswi::util::SafeList<std::shared_ptr<ProcEntry>> m_list {"RegistryList"};
    long m_pollInterval = 3000000;
};

} // namespace tkm::monitor
