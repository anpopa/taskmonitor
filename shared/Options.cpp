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
: m_configFile(std::make_shared<bswi::kf::KeyFile>(configFile))
{
  if (m_configFile->parseFile() != 0) {
    logWarn() << "Fail to parse config file: " << configFile;
    m_configFile.reset();
  }
}

auto Options::getFor(Key key) -> string const
{
  switch (key) {
  case Key::RuntimeDirectory:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("monitor", -1, "RuntimeDirectory");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::RuntimeDirectory));
    }
    return tkmDefaults.getFor(Defaults::Default::RuntimeDirectory);
  case Key::ContainersPath:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("monitor", -1, "ContainersPath");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ContainersPath));
    }
    return tkmDefaults.getFor(Defaults::Default::ContainersPath);
  case Key::RxBufferSize:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("monitor", -1, "RxBufferSize");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::RxBufferSize));
    }
    return tkmDefaults.getFor(Defaults::Default::RxBufferSize);
  case Key::TxBufferSize:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("monitor", -1, "TxBufferSize");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::TxBufferSize));
    }
    return tkmDefaults.getFor(Defaults::Default::TxBufferSize);
  case Key::MsgBufferSize:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("monitor", -1, "MsgBufferSize");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::MsgBufferSize));
    }
    return tkmDefaults.getFor(Defaults::Default::MsgBufferSize);
  case Key::ProdModeFastLaneInt:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("production-mode", -1, "FastLaneInterval");

      try {
        auto interval =
            std::stoul(prop.value_or(tkmDefaults.getFor(Defaults::Default::ProdModeFastLaneInt)));
        if (interval < 1000000) {
          return tkmDefaults.getFor(Defaults::Default::ProdModeFastLaneInt);
        }
      } catch (...) {
        return tkmDefaults.getFor(Defaults::Default::ProdModeFastLaneInt);
      }

      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProdModeFastLaneInt));
    }
    return tkmDefaults.getFor(Defaults::Default::ProdModeFastLaneInt);
  case Key::ProdModePaceLaneInt:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("production-mode", -1, "PaceLaneInterval");

      try {
        auto interval =
            std::stoul(prop.value_or(tkmDefaults.getFor(Defaults::Default::ProdModePaceLaneInt)));
        if (interval < 1000000) {
          return tkmDefaults.getFor(Defaults::Default::ProdModePaceLaneInt);
        }
      } catch (...) {
        return tkmDefaults.getFor(Defaults::Default::ProdModePaceLaneInt);
      }

      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProdModePaceLaneInt));
    }
    return tkmDefaults.getFor(Defaults::Default::ProdModePaceLaneInt);
  case Key::ProdModeSlowLaneInt:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("production-mode", -1, "SlowLaneInterval");

      try {
        auto interval =
            std::stoul(prop.value_or(tkmDefaults.getFor(Defaults::Default::ProdModeSlowLaneInt)));
        if (interval < 1000000) {
          return tkmDefaults.getFor(Defaults::Default::ProdModeSlowLaneInt);
        }
      } catch (...) {
        return tkmDefaults.getFor(Defaults::Default::ProdModeSlowLaneInt);
      }

      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProdModeSlowLaneInt));
    }
    return tkmDefaults.getFor(Defaults::Default::ProdModeSlowLaneInt);
  case Key::ProfModeFastLaneInt:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("profiling-mode", -1, "FastLaneInterval");

      try {
        auto interval =
            std::stoul(prop.value_or(tkmDefaults.getFor(Defaults::Default::ProfModeFastLaneInt)));
        if (interval < 1000000) {
          return tkmDefaults.getFor(Defaults::Default::ProfModeFastLaneInt);
        }
      } catch (...) {
        return tkmDefaults.getFor(Defaults::Default::ProfModeFastLaneInt);
      }

      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProfModeFastLaneInt));
    }
    return tkmDefaults.getFor(Defaults::Default::ProfModeFastLaneInt);
  case Key::ProfModePaceLaneInt:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("profiling-mode", -1, "PaceLaneInterval");

      try {
        auto interval =
            std::stoul(prop.value_or(tkmDefaults.getFor(Defaults::Default::ProfModePaceLaneInt)));
        if (interval < 1000000) {
          return tkmDefaults.getFor(Defaults::Default::ProfModePaceLaneInt);
        }
      } catch (...) {
        return tkmDefaults.getFor(Defaults::Default::ProfModePaceLaneInt);
      }

      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProfModePaceLaneInt));
    }
    return tkmDefaults.getFor(Defaults::Default::ProfModePaceLaneInt);
  case Key::ProfModeSlowLaneInt:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("profiling-mode", -1, "SlowLaneInterval");

      try {
        auto interval =
            std::stoul(prop.value_or(tkmDefaults.getFor(Defaults::Default::ProfModeSlowLaneInt)));
        if (interval < 1000000) {
          return tkmDefaults.getFor(Defaults::Default::ProfModeSlowLaneInt);
        }
      } catch (...) {
        return tkmDefaults.getFor(Defaults::Default::ProfModeSlowLaneInt);
      }

      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProfModeSlowLaneInt));
    }
    return tkmDefaults.getFor(Defaults::Default::ProfModeSlowLaneInt);
  case Key::ProfModeIfPath:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("monitor", -1, "ProfModeIfPath");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProfModeIfPath));
    }
    return tkmDefaults.getFor(Defaults::Default::ProfModeIfPath);
  case Key::SelfLowerPriority:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("monitor", -1, "SelfLowerPriority");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::SelfLowerPriority));
    }
    return tkmDefaults.getFor(Defaults::Default::SelfLowerPriority);
  case Key::ReadProcAtInit:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("monitor", -1, "ReadProcAtInit");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ReadProcAtInit));
    }
    return tkmDefaults.getFor(Defaults::Default::ReadProcAtInit);
  case Key::EnableProcAcct:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("monitor", -1, "EnableProcAcct");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::EnableProcAcct));
    }
    return tkmDefaults.getFor(Defaults::Default::EnableProcAcct);
  case Key::EnableTCPServer:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("monitor", -1, "EnableTCPServer");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::EnableTCPServer));
    }
    return tkmDefaults.getFor(Defaults::Default::EnableTCPServer);
  case Key::EnableUDSServer:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("monitor", -1, "EnableUDSServer");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::EnableUDSServer));
    }
    return tkmDefaults.getFor(Defaults::Default::EnableUDSServer);
  case Key::EnableStartupData:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("monitor", -1, "EnableStartupData");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::EnableStartupData));
    }
    return tkmDefaults.getFor(Defaults::Default::EnableStartupData);
  case Key::StartupDataCleanupTime:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("profiling-mode", -1, "StartupDataCleanupTime");

      try {
        auto interval = std::stoul(
            prop.value_or(tkmDefaults.getFor(Defaults::Default::StartupDataCleanupTime)));
        if (interval < 1000000) {
          return tkmDefaults.getFor(Defaults::Default::StartupDataCleanupTime);
        }
      } catch (...) {
        return tkmDefaults.getFor(Defaults::Default::StartupDataCleanupTime);
      }

      return prop.value_or(tkmDefaults.getFor(Defaults::Default::StartupDataCleanupTime));
    }
    return tkmDefaults.getFor(Defaults::Default::StartupDataCleanupTime);
  case Key::TCPServerAddress:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("tcpserver", -1, "ServerAddress");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::TCPServerAddress));
    }
    return tkmDefaults.getFor(Defaults::Default::TCPServerAddress);
  case Key::TCPServerPort:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("tcpserver", -1, "ServerPort");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::TCPServerPort));
    }
    return tkmDefaults.getFor(Defaults::Default::TCPServerPort);
  case Key::UDSServerSocketPath:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("udsserver", -1, "SocketPath");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::UDSServerSocketPath));
    }
    return tkmDefaults.getFor(Defaults::Default::UDSServerSocketPath);
  default:
    logError() << "Unknown option key";
    break;
  }

  throw std::runtime_error("Cannot provide option for key");
}

} // namespace tkm::monitor
