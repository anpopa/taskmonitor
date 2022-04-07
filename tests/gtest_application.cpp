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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "../source/Application.h"
#include "gtest/gtest.h"

using namespace std;
using namespace tkm::monitor;

class GTestApplication : public ::testing::Test
{
protected:
  GTestApplication();
  virtual ~GTestApplication();

  virtual void SetUp();
  virtual void TearDown();

protected:
  unique_ptr<Application> m_app;
};

GTestApplication::GTestApplication()
{
  m_app = make_unique<Application>(
      "TKM", "TaskMonitor Application", tkmDefaults.getFor(Defaults::Default::ConfPath));
}

GTestApplication::~GTestApplication() {}

void GTestApplication::SetUp() {}

void GTestApplication::TearDown() {}

TEST_F(GTestApplication, parse)
{
  if (TaskMonitor()->hasConfigFile()) {
    TaskMonitor()->getConfigFile()->printStdOutput();
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
