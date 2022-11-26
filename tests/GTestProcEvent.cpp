/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcEvent Class Unit Tets
 * @details   GTests for ProcEvent class
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

#include "../source/ProcEvent.h"
#include "../tests/dummy/Application.h"

using namespace tkm::monitor;

static std::unique_ptr<Application> app = nullptr;

static void appRun()
{
  App()->run();
}

class GTestProcEvent : public ::testing::Test
{
protected:
  GTestProcEvent();
  virtual ~GTestProcEvent();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
};

GTestProcEvent::GTestProcEvent()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  if (getuid() == 0) {
    m_ownThread = std::make_unique<std::thread>(appRun);
  }
}

GTestProcEvent::~GTestProcEvent()
{
  if (getuid() == 0) {
    App()->stop();
    m_ownThread->join();
  }
  app.reset();
}

void GTestProcEvent::SetUp()
{
  if (getuid() == 0) {
    // Create ProcRegistry
    App()->m_procRegistry = std::make_shared<ProcRegistry>(App()->getOptions());
    App()->getProcRegistry()->setEventSource();

    // Create ProcEvent
    App()->m_procEvent = std::make_shared<ProcEvent>(App()->getOptions());
    App()->getProcEvent()->setEventSource();
  }
}

void GTestProcEvent::TearDown()
{
  if (getuid() == 0) {
    App()->getProcRegistry()->setEventSource(false);
    App()->getProcEvent()->setEventSource(false);
  }
}

TEST_F(GTestProcEvent, Update)
{
  if (getuid() == 0) {
    const auto forkCount = App()->getProcEvent()->getProcEventData().fork_count();
    const auto execCount = App()->getProcEvent()->getProcEventData().exec_count();
    const auto exitCount = App()->getProcEvent()->getProcEventData().exit_count();
    const auto uidCount = App()->getProcEvent()->getProcEventData().uid_count();
    const auto gidCount = App()->getProcEvent()->getProcEventData().gid_count();

    system("uname -a");
    sleep(1);

    EXPECT_GT(App()->getProcEvent()->getProcEventData().fork_count(), forkCount);
    EXPECT_GT(App()->getProcEvent()->getProcEventData().exec_count(), execCount);
    EXPECT_GT(App()->getProcEvent()->getProcEventData().exit_count(), exitCount);
    EXPECT_EQ(App()->getProcEvent()->getProcEventData().uid_count(), uidCount);
    EXPECT_EQ(App()->getProcEvent()->getProcEventData().gid_count(), gidCount);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
