/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Application Class Unit Tets
 * @details   GTests for Application class
 *-
 */

#include <fstream>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <utility>

#include "../source/Application.h"

using namespace tkm::monitor;

static void appStop()
{
  sleep(3);
  App()->stop();
}

class GTestApplication : public ::testing::Test
{
protected:
  GTestApplication() = default;
  virtual ~GTestApplication();

protected:
  std::unique_ptr<Application> m_app = nullptr;
  std::unique_ptr<std::thread> m_stopThread = nullptr;
};

GTestApplication::~GTestApplication() {}

TEST_F(GTestApplication, CreateAndStopApplication_NoConfig)
{
  EXPECT_NO_THROW(m_app =
                      std::make_unique<Application>("TaskMonitor", "TaskMonitor", std::string()));
  EXPECT_EQ(m_app->getOptions()->hasConfigFile(), false);

  m_stopThread = std::make_unique<std::thread>(appStop);
  EXPECT_NO_THROW(App()->run());
  m_stopThread->join();
  m_app.reset();
}

TEST_F(GTestApplication, CreateAndStopApplication)
{
  EXPECT_NO_THROW(m_app = std::make_unique<Application>(
                      "TaskMonitor", "TaskMonitor", "assets/taskmonitor.conf"));
  EXPECT_EQ(m_app->getOptions()->hasConfigFile(), true);

  m_stopThread = std::make_unique<std::thread>(appStop);
  EXPECT_NO_THROW(App()->run());
  m_stopThread->join();
  m_app.reset();
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
