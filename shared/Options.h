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

#include <any>
#include <string>

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/KeyFile.h"
#include "../bswinfra/source/Logger.h"

namespace tkm::monitor
{

class Options
{
public:
    enum class Key { 
        PollInterval,
        RxBufferSize,
        TxBufferSize,
        MsgBufferSize,
        ReadProcAtInit,
        WithCPU, 
        WithMemory, 
        WithContext,
        WithIO,
        WithSwap,
        WithReclaim,
        WithTrashing
        };

public:
    Options(const std::string &configFile);

    auto getFor(Key key) -> std::string const;
    auto hasConfigFile() -> bool { return m_configFile != nullptr; }
    auto getConfigFile() -> std::shared_ptr<bswi::kf::KeyFile> { return m_configFile; }

private:
    std::shared_ptr<bswi::kf::KeyFile> m_configFile = nullptr;
};

} // namespace tkm::monitor
