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
  case Key::RuntimeDirectory:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("monitor", -1, "RuntimeDirectory");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::RuntimeDirectory));
    }
    return tkmDefaults.getFor(Defaults::Default::RuntimeDirectory);
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
  case Key::StatPollInterval:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("sysstat", -1, "PollInterval");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::StatPollInterval));
    }
    return tkmDefaults.getFor(Defaults::Default::StatPollInterval);
  case Key::MemPollInterval:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("meminfo", -1, "PollInterval");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::MemPollInterval));
    }
    return tkmDefaults.getFor(Defaults::Default::MemPollInterval);
  case Key::ProcPollInterval:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("procacct", -1, "PollInterval");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProcPollInterval));
    }
    return tkmDefaults.getFor(Defaults::Default::ProcPollInterval);
  case Key::ProcEventPollInterval:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("procevent", -1, "PollInterval");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ProcEventPollInterval));
    }
    return tkmDefaults.getFor(Defaults::Default::ProcEventPollInterval);
  case Key::ReadProcAtInit:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("prrocacct", -1, "ReadProcAtInit");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::ReadProcAtInit));
    }
    return tkmDefaults.getFor(Defaults::Default::ReadProcAtInit);
  case Key::PressurePollInterval:
    if (hasConfigFile()) {
      const optional<string> prop =
          m_configFile->getPropertyValue("presure", -1, "PressurePollInterval");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::PressurePollInterval));
    }
    return tkmDefaults.getFor(Defaults::Default::PressurePollInterval);
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
  case Key::TCPServerStartIfPath:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("tcpserver", -1, "StartIfPath");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::TCPServerStartIfPath));
    }
    return tkmDefaults.getFor(Defaults::Default::TCPServerStartIfPath);
  case Key::UDSServerSocketPath:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("udsserver", -1, "SocketPath");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::UDSServerSocketPath));
    }
    return tkmDefaults.getFor(Defaults::Default::UDSServerSocketPath);
  case Key::UDSServerStartIfPath:
    if (hasConfigFile()) {
      const optional<string> prop = m_configFile->getPropertyValue("udsserver", -1, "StartIfPath");
      return prop.value_or(tkmDefaults.getFor(Defaults::Default::UDSServerStartIfPath));
    }
    return tkmDefaults.getFor(Defaults::Default::UDSServerStartIfPath);
  default:
    logError() << "Unknown option key";
    break;
  }

  throw std::runtime_error("Cannot provide option for key");
}

} // namespace tkm::monitor
