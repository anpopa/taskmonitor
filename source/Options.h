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

#include "Defaults.h"

#include "../bswinfra/source/KeyFile.h"

namespace tkm::monitor
{

class Options
{
public:
  enum class Key {
    RuntimeDirectory,
    ContainersPath,
    RxBufferSize,
    TxBufferSize,
    MsgBufferSize,
    ProdModeFastLaneInt,
    ProdModePaceLaneInt,
    ProdModeSlowLaneInt,
    ProfModeFastLaneInt,
    ProfModePaceLaneInt,
    ProfModeSlowLaneInt,
    ProfModeIfPath,
    SelfLowerPriority,
    ReadProcAtInit,
    EnableProcEvent,
    EnableProcAcct,
    EnableTCPServer,
    EnableUDSServer,
    EnableStartupData,
    UpdateOnProcEvent,
    StartupDataCleanupTime,
    TCPServerAddress,
    TCPServerPort,
    UDSServerSocketPath,
    CollectorInactiveTimeout,
  };

public:
  explicit Options(const std::string &configFile);

  auto getFor(Key key) -> std::string const;
  bool hasConfigFile() { return m_configFile != nullptr; }
  auto getConfigFile() -> std::shared_ptr<bswi::kf::KeyFile> { return m_configFile; }

private:
  std::shared_ptr<bswi::kf::KeyFile> m_configFile = nullptr;
};

} // namespace tkm::monitor
