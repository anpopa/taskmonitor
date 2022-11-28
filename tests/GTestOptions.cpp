/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Options Class Unit Tets
 * @details   GTests for Options class
 *-
 */

#include <fstream>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "../source/Options.h"

using namespace tkm::monitor;

class GTestOptions : public ::testing::Test
{
protected:
  GTestOptions() = default;
  virtual ~GTestOptions();
};

GTestOptions::~GTestOptions() {}

TEST_F(GTestOptions, Options_NoConfig)
{
  std::unique_ptr<Options> opts = nullptr;

  opts = std::make_unique<Options>(std::string());
  EXPECT_EQ(opts->hasConfigFile(), false);
}

TEST_F(GTestOptions, Options_Defaults)
{
  std::unique_ptr<Options> opts = nullptr;

  opts = std::make_unique<Options>(std::string());

  EXPECT_STRCASEEQ(opts->getFor(Options::Key::RuntimeDirectory).c_str(),
                   tkmDefaults.getFor(Defaults::Default::RuntimeDirectory).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ContainersPath).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ContainersPath).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::RxBufferSize).c_str(),
                   tkmDefaults.getFor(Defaults::Default::RxBufferSize).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::TxBufferSize).c_str(),
                   tkmDefaults.getFor(Defaults::Default::TxBufferSize).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::MsgBufferSize).c_str(),
                   tkmDefaults.getFor(Defaults::Default::MsgBufferSize).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModeFastLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModeFastLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModePaceLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModePaceLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModeSlowLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModeSlowLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeFastLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModeFastLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModePaceLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModePaceLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeSlowLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModeSlowLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeIfPath).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModeIfPath).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::SelfLowerPriority).c_str(),
                   tkmDefaults.getFor(Defaults::Default::SelfLowerPriority).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ReadProcAtInit).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ReadProcAtInit).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableProcEvent).c_str(),
                   tkmDefaults.getFor(Defaults::Default::EnableProcEvent).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::UpdateOnProcEvent).c_str(),
                   tkmDefaults.getFor(Defaults::Default::UpdateOnProcEvent).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableProcAcct).c_str(),
                   tkmDefaults.getFor(Defaults::Default::EnableProcAcct).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableTCPServer).c_str(),
                   tkmDefaults.getFor(Defaults::Default::EnableTCPServer).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableUDSServer).c_str(),
                   tkmDefaults.getFor(Defaults::Default::EnableUDSServer).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableStartupData).c_str(),
                   tkmDefaults.getFor(Defaults::Default::EnableStartupData).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::StartupDataCleanupTime).c_str(),
                   tkmDefaults.getFor(Defaults::Default::StartupDataCleanupTime).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::TCPServerAddress).c_str(),
                   tkmDefaults.getFor(Defaults::Default::TCPServerAddress).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::TCPServerPort).c_str(),
                   tkmDefaults.getFor(Defaults::Default::TCPServerPort).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::UDSServerSocketPath).c_str(),
                   tkmDefaults.getFor(Defaults::Default::UDSServerSocketPath).c_str());
}

TEST_F(GTestOptions, Options_HasConfig)
{
  std::unique_ptr<Options> opts = nullptr;

  opts = std::make_unique<Options>("assets/taskmonitor.conf");
  EXPECT_EQ(opts->hasConfigFile(), true);
}

TEST_F(GTestOptions, Options_NotDefaults)
{
  std::unique_ptr<Options> opts = nullptr;

  opts = std::make_unique<Options>("assets/taskmonitor_var0.conf");
  EXPECT_EQ(opts->hasConfigFile(), true);

  EXPECT_STRCASEEQ(opts->getFor(Options::Key::RuntimeDirectory).c_str(), "/tmp/taskmonitor");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ContainersPath).c_str(), "/tmp/lib/lxc");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::RxBufferSize).c_str(), "262144");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::TxBufferSize).c_str(), "262144");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::MsgBufferSize).c_str(), "524288");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModeFastLaneInt).c_str(), "20000000");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModePaceLaneInt).c_str(), "40000000");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModeSlowLaneInt).c_str(), "50000000");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeFastLaneInt).c_str(), "2000000");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModePaceLaneInt).c_str(), "6000000");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeSlowLaneInt).c_str(), "20000000");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeIfPath).c_str(), "/tmp/taskmonitor");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::SelfLowerPriority).c_str(), "false");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ReadProcAtInit).c_str(), "false");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableProcEvent).c_str(), "false");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::UpdateOnProcEvent).c_str(), "false");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableProcAcct).c_str(), "false");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableTCPServer).c_str(), "false");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableUDSServer).c_str(), "true");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::EnableStartupData).c_str(), "true");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::StartupDataCleanupTime).c_str(), "70000000");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::TCPServerAddress).c_str(), "localhost");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::TCPServerPort).c_str(), "3358");
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::UDSServerSocketPath).c_str(),
                   "/tmp/taskmonitor.sock");
}

TEST_F(GTestOptions, Options_SmallIntervals_UseDefaults)
{
  std::unique_ptr<Options> opts = nullptr;

  opts = std::make_unique<Options>("assets/taskmonitor_var1.conf");
  EXPECT_EQ(opts->hasConfigFile(), true);

  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModeFastLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModeFastLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModePaceLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModePaceLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModeSlowLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModeSlowLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeFastLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModeFastLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModePaceLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModePaceLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeSlowLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModeSlowLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::StartupDataCleanupTime).c_str(),
                   tkmDefaults.getFor(Defaults::Default::StartupDataCleanupTime).c_str());
}

TEST_F(GTestOptions, Options_InvalidIntervals_UseDefaults)
{
  std::unique_ptr<Options> opts = nullptr;

  opts = std::make_unique<Options>("assets/taskmonitor_var2.conf");
  EXPECT_EQ(opts->hasConfigFile(), true);

  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModeFastLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModeFastLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModePaceLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModePaceLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProdModeSlowLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProdModeSlowLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeFastLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModeFastLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModePaceLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModePaceLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::ProfModeSlowLaneInt).c_str(),
                   tkmDefaults.getFor(Defaults::Default::ProfModeSlowLaneInt).c_str());
  EXPECT_STRCASEEQ(opts->getFor(Options::Key::StartupDataCleanupTime).c_str(),
                   tkmDefaults.getFor(Defaults::Default::StartupDataCleanupTime).c_str());
}

TEST_F(GTestOptions, InvalidKey)
{
  std::unique_ptr<Options> opts = nullptr;

  opts = std::make_unique<Options>("assets/taskmonitor.conf");
  EXPECT_EQ(opts->hasConfigFile(), true);

  EXPECT_THROW(opts->getFor(static_cast<Options::Key>(-1)), std::runtime_error);
}

TEST_F(GTestOptions, Options_Blacklist)
{
  std::unique_ptr<Options> opts = nullptr;

  opts = std::make_unique<Options>("assets/taskmonitor.conf");
  EXPECT_EQ(opts->hasConfigFile(), true);

  const std::vector<bswi::kf::Property> props =
      opts->getConfigFile()->getProperties("blacklist", -1);

  EXPECT_FALSE(props.empty());

  std::string name = "kworker";
  bool found = false;

  for (const auto &prop : props) {
    if (name.find(prop.key) != std::string::npos) {
      found = true;
    }
  }
  EXPECT_TRUE(found);

  name = "cgroupify";
  found = false;

  for (const auto &prop : props) {
    if (name.find(prop.key) != std::string::npos) {
      found = true;
    }
  }
  EXPECT_TRUE(found);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
