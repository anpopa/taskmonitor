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

#include "Options.h"
#include "Defaults.h"

using namespace std;

namespace tkm::monitor
{

Options::Options(const string &configFile)
{
    m_configFile = std::make_shared<bswi::kf::KeyFile>(configFile);
    if (m_configFile->parseFile() != 0) {
        logWarn() << "Fail to parse config file: " << configFile;
        m_configFile.reset();
    }
}

auto Options::getFor(Key key) -> string const
{
    switch (key) {
    case Key::StatPollInterval:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("systats", -1, "PollInterval");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::StatPollInterval));
        }
        return tkmDefaults.getFor(Defaults::Default::StatPollInterval);
    case Key::ProcPollInterval:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "PollInterval");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProcPollInterval));
        }
        return tkmDefaults.getFor(Defaults::Default::ProcPollInterval);
    case Key::PressurePollInterval:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("monitor", -1, "PressurePollInterval");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::PressurePollInterval));
        }
        return tkmDefaults.getFor(Defaults::Default::PressurePollInterval);
    case Key::RxBufferSize:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("monitor", -1, "RxBufferSize");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::RxBufferSize));
        }
        return tkmDefaults.getFor(Defaults::Default::RxBufferSize);
    case Key::TxBufferSize:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("monitor", -1, "TxBufferSize");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::TxBufferSize));
        }
        return tkmDefaults.getFor(Defaults::Default::TxBufferSize);
    case Key::MsgBufferSize:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("monitor", -1, "MsgBufferSize");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::MsgBufferSize));
        }
        return tkmDefaults.getFor(Defaults::Default::MsgBufferSize);
    case Key::ReadProcAtInit:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "ReadProcAtInit");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::ReadProcAtInit));
        }
        return tkmDefaults.getFor(Defaults::Default::ReadProcAtInit);
    case Key::EnableSysStat:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("monitor", -1, "EnableSysStat");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::EnableSysStat));
        }
        return tkmDefaults.getFor(Defaults::Default::EnableSysStat);
    case Key::EnableNetServer:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("monitor", -1, "EnableNetServer");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::EnableNetServer));
        }
        return tkmDefaults.getFor(Defaults::Default::EnableNetServer);
    case Key::EnableSysPressure:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("monitor", -1, "EnableSysPressure");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::EnableSysPressure));
        }
        return tkmDefaults.getFor(Defaults::Default::EnableSysPressure);
    case Key::NetServerAddress:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("netserver", -1, "ServerAddress");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::NetServerAddress));
        }
        return tkmDefaults.getFor(Defaults::Default::NetServerAddress);
    case Key::NetServerPort:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("netserver", -1, "ServerPort");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::NetServerPort));
        }
        return tkmDefaults.getFor(Defaults::Default::NetServerPort);
    case Key::WithCPU:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "WithCPU");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::WithCPU));
        }
        return tkmDefaults.getFor(Defaults::Default::WithCPU);
    case Key::WithMemory:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "WithMemory");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::WithMemory));
        }
        return tkmDefaults.getFor(Defaults::Default::WithMemory);
    case Key::WithContext:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "WithContext");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::WithContext));
        }
        return tkmDefaults.getFor(Defaults::Default::WithContext);
    case Key::WithIO:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "WithIO");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::WithIO));
        }
        return tkmDefaults.getFor(Defaults::Default::WithIO);
    case Key::WithSwap:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "WithSwap");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::WithSwap));
        }
        return tkmDefaults.getFor(Defaults::Default::WithSwap);
    case Key::WithReclaim:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "WithReclaim");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::WithReclaim));
        }
        return tkmDefaults.getFor(Defaults::Default::WithReclaim);
    case Key::WithTrashing:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("process", -1, "WithTrashing");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::WithTrashing));
        }
        return tkmDefaults.getFor(Defaults::Default::WithTrashing);
    case Key::PressureWithCPU:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("pressure", -1, "WithCPU");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::PressureWithCPU));
        }
        return tkmDefaults.getFor(Defaults::Default::PressureWithCPU);
    case Key::PressureWithMemory:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("pressure", -1, "WithMemory");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::PressureWithMemory));
        }
        return tkmDefaults.getFor(Defaults::Default::PressureWithMemory);
    case Key::PressureWithIO:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("pressure", -1, "WithIO");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::PressureWithIO));
        }
        return tkmDefaults.getFor(Defaults::Default::PressureWithIO);
    default:
        logError() << "Unknown option key";
        break;
    }

    throw std::runtime_error("Cannot provide option for key");
}

} // namespace tkm::monitor
