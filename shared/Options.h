/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Options class
 * @details   Runtime options with defaults
 *-
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
        StatPollInterval,
        ProcPollInterval,
        PressurePollInterval,
        RxBufferSize,
        TxBufferSize,
        MsgBufferSize,
        ReadProcAtInit,
        EnableSysPressure,
        EnableSysStat,
        WithCPU, 
        WithMemory, 
        WithContext,
        WithIO,
        WithSwap,
        WithReclaim,
        WithTrashing,
        PressureWithCPU,
        PressureWithMemory,
        PressureWithIO
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
