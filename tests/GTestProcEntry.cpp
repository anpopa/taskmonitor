/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcEntry Class Unit Tets
 * @details   GTests for ProcEntry class
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

#include "../tests/dummy/Application.h"

using namespace tkm::monitor;

static std::unique_ptr<Application> app = nullptr;

static void appRun()
{
  App()->run();
}

class GTestProcEntry : public ::testing::Test
{
protected:
  GTestProcEntry();
  virtual ~GTestProcEntry();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
};

GTestProcEntry::GTestProcEntry()
{
  app = std::make_unique<Application>(
      "TKM", "TaskMonitor Application", "assets/taskmonitor_var0.conf");
  if (getuid() == 0) {
    m_ownThread = std::make_unique<std::thread>(appRun);
  }
}

GTestProcEntry::~GTestProcEntry()
{
  if (getuid() == 0) {
    App()->stop();
    m_ownThread->join();
  }
  app.reset();
}

void GTestProcEntry::SetUp()
{
  // Create ProcRegistry
  App()->m_procRegistry = std::make_shared<ProcRegistry>(App()->getOptions());
  App()->getProcRegistry()->setEventSource();

#ifdef WITH_PROC_ACCT
  App()->m_procAcct = std::make_shared<ProcAcct>(App()->getOptions());
  App()->getProcAcct()->setEventSource();
#endif
}

void GTestProcEntry::TearDown()
{
  App()->getProcRegistry()->setEventSource(false);
#ifdef WITH_PROC_ACCT
  App()->getProcAcct()->setEventSource(false);
#endif
}

TEST_F(GTestProcEntry, Update)
{
  App()->m_procAcctCollectorCounter = 1;

  EXPECT_EQ(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);
  App()->getProcRegistry()->addProcEntry(getpid()); // Add self to proc list
  EXPECT_NE(App()->getProcRegistry()->getProcEntry(getpid()), nullptr);

  const std::shared_ptr<ProcEntry> testEntry = App()->getProcRegistry()->getProcEntry(getpid());
  EXPECT_NE(testEntry, nullptr);

  testEntry->update();
  sleep(1);

  const auto infCpuTime = testEntry->getData().cpu_time();
  EXPECT_STRCASEEQ(testEntry->getName().c_str(), "GTestProcEntry");
  EXPECT_STRCASEEQ(testEntry->getData().comm().c_str(), "GTestProcEntry");
  EXPECT_EQ(testEntry->getData().pid(), getpid());

  if (getuid() == 0) {
#ifdef WITH_PROC_ACCT
    const auto accCpuTime = testEntry->getAcct().cpu().cpu_count();
    EXPECT_EQ(testEntry->getAcct().ac_pid(), getpid());
    EXPECT_STRCASEEQ(testEntry->getAcct().ac_comm().c_str(), "GTestProcEntry");
#endif
    testEntry->update(tkmDefaults.valFor(Defaults::Val::ProcAcct));
#ifdef WITH_PROC_ACCT
    EXPECT_GT(testEntry->getAcct().cpu().cpu_count(), accCpuTime);
#endif
    EXPECT_EQ(testEntry->getData().cpu_time(), infCpuTime);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
