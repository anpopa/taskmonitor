/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcAcct Class Unit Tets
 * @details   GTests for ProcAcct class
 *-
 */

#include <fstream>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taskmonitor/taskmonitor.h>
#include <thread>
#include <utility>

#include "../source/ProcAcct.h"
#include "../tests/dummy/Application.h"

using namespace tkm::monitor;

static std::unique_ptr<Application> app = nullptr;

static void appRun()
{
  App()->run();
}

class GTestProcAcct : public ::testing::Test
{
protected:
  GTestProcAcct();
  virtual ~GTestProcAcct();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
};

GTestProcAcct::GTestProcAcct()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  if (getuid() == 0) {
    m_ownThread = std::make_unique<std::thread>(appRun);
  }
}

GTestProcAcct::~GTestProcAcct()
{
  if (getuid() == 0) {
    App()->stop();
    m_ownThread->join();
  }
  app.reset();
}

void GTestProcAcct::SetUp()
{
  // Create ProcRegistry
  App()->m_procRegistry = std::make_shared<ProcRegistry>(App()->getOptions());
  App()->getProcRegistry()->setEventSource();

  // Create ProcAcct
  App()->m_procAcct = std::make_shared<ProcAcct>(App()->getOptions());
  App()->getProcAcct()->setEventSource();
}

void GTestProcAcct::TearDown()
{
  App()->getProcRegistry()->setEventSource(false);
  App()->getProcAcct()->setEventSource(false);
}

TEST_F(GTestProcAcct, RequestAccounting)
{
  if (getuid() == 0) {
    EXPECT_EQ(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);
    App()->getProcRegistry()->addProcEntry(getpid()); // Add self to proc list
    EXPECT_NE(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);

    EXPECT_TRUE(App()->getProcAcct()->requestTaskAcct(getpid()));
    sleep(1);

    const std::shared_ptr<ProcEntry> testEntry = App()->getProcRegistry()->getProcEntry(getpid());
    EXPECT_EQ(testEntry->getAcct().ac_pid(), getpid());
    EXPECT_STRCASEEQ(testEntry->getAcct().ac_comm().c_str(), "GTestProcAcct");
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
