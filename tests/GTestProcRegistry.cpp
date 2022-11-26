/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcRegistry Class Unit Tets
 * @details   GTests for ProcRegistry class
 *-
 */

#include <cmath>
#include <fstream>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taskmonitor/taskmonitor.h>
#include <thread>
#include <utility>

#include "../source/ProcRegistry.h"
#include "../tests/dummy/Application.h"

using namespace tkm::monitor;

static std::unique_ptr<Application> app = nullptr;

static void appRun()
{
  App()->run();
}

class GTestProcRegistry : public ::testing::Test
{
protected:
  GTestProcRegistry();
  virtual ~GTestProcRegistry();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
};

GTestProcRegistry::GTestProcRegistry()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  if (getuid() == 0) {
    m_ownThread = std::make_unique<std::thread>(appRun);
  }
}

GTestProcRegistry::~GTestProcRegistry()
{
  if (getuid() == 0) {
    App()->stop();
    m_ownThread->join();
  }
  app.reset();
}

void GTestProcRegistry::SetUp()
{
  if (getuid() == 0) {
    // Create ProcRegistry
    App()->m_procRegistry = std::make_shared<ProcRegistry>(App()->getOptions());
    App()->getProcRegistry()->setEventSource();

    // Create ProcAcct
    App()->m_procAcct = std::make_shared<ProcAcct>(App()->getOptions());
    App()->getProcAcct()->setEventSource();
  }
}

void GTestProcRegistry::TearDown()
{
  if (getuid() == 0) {
    App()->getProcRegistry()->setEventSource(false);
    App()->getProcAcct()->setEventSource(false);
  }
}

TEST_F(GTestProcRegistry, UpdateProcessList)
{
  App()->m_procAcctCollectorCounter = 1;

  if (getuid() == 0) {
    EXPECT_EQ(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);

    App()->getProcRegistry()->addProcEntry(getpid());
    EXPECT_NE(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);

    const std::shared_ptr<ProcEntry> testEntry = App()->getProcRegistry()->getProcEntry(getpid());
    EXPECT_NE(testEntry, nullptr);

    EXPECT_EQ(App()->getProcRegistry()->update(), true);
    EXPECT_EQ(testEntry->getAcct().ac_pid(), getpid());
    EXPECT_STRCASEEQ(testEntry->getName().c_str(), "GTestProcRegist");

    App()->getProcRegistry()->updProcEntry(getpid());
    EXPECT_NE(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);

    App()->getProcRegistry()->remProcEntry(getpid());
    sleep(1);
    EXPECT_EQ(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);

    App()->getProcRegistry()->addProcEntry(getpid());
    EXPECT_NE(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);
    App()->getProcRegistry()->update(ProcRegistry::UpdateLane::Fast);
    App()->getProcRegistry()->update(ProcRegistry::UpdateLane::Slow);

    App()->getProcRegistry()->remProcEntry("GTestProcRegist", true);
    EXPECT_EQ(App()->getProcRegistry()->getProcEntry("GTestProcRegist"), nullptr);
    EXPECT_EQ(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);
  }
}

TEST_F(GTestProcRegistry, Request_CommitProcList)
{
  App()->m_procAcctCollectorCounter = 1;

  if (getuid() == 0) {
    EXPECT_EQ(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);
    App()->getProcRegistry()->addProcEntry(getpid());
    EXPECT_NE(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);

    const std::shared_ptr<ProcEntry> testEntry = App()->getProcRegistry()->getProcEntry(getpid());
    EXPECT_NE(testEntry, nullptr);

    EXPECT_EQ(App()->getProcRegistry()->update(), true);
    EXPECT_EQ(testEntry->getAcct().ac_pid(), getpid());
    EXPECT_STRCASEEQ(testEntry->getName().c_str(), "GTestProcRegist");

    App()->getProcRegistry()->remProcEntry(getpid());
    sleep(1);
    EXPECT_EQ(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);
  }
}

TEST_F(GTestProcRegistry, Request_InitFromProc)
{
  App()->m_procAcctCollectorCounter = 1;

  if (getuid() == 0) {
    EXPECT_EQ(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);

    App()->getProcRegistry()->initFromProc();

    const std::shared_ptr<ProcEntry> testEntry = App()->getProcRegistry()->getProcEntry(getpid());
    EXPECT_NE(testEntry, nullptr);

    EXPECT_EQ(App()->getProcRegistry()->update(), true);
    EXPECT_EQ(testEntry->getAcct().ac_pid(), getpid());
    EXPECT_STRCASEEQ(testEntry->getName().c_str(), "GTestProcRegist");
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
