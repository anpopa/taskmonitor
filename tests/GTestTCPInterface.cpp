/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     TCPServer and TCPCollector Unit Tets
 * @details   GTestTCPInterface for TCPServer and TCPCollector class
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
#include "../tests/dummy/Reader.h"

using namespace tkm::monitor;

static std::unique_ptr<Application> app = nullptr;

static void appRun()
{
  App()->run();
}

class GTestTCPInterface : public ::testing::Test
{
protected:
  GTestTCPInterface();
  virtual ~GTestTCPInterface();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
  std::shared_ptr<Reader> m_reader = nullptr;
};

GTestTCPInterface::GTestTCPInterface()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  m_ownThread = std::make_unique<std::thread>(appRun);
}

GTestTCPInterface::~GTestTCPInterface()
{
  App()->stop();
  m_ownThread->join();
  app.reset();
}

void GTestTCPInterface::SetUp()
{
  App()->m_netServer = std::make_shared<TCPServer>(App()->getOptions());
  App()->getTCPServer()->setEventSource(true);

  m_reader = std::make_shared<Reader>(App()->getOptions(), Reader::Type::INET);
  m_reader->setEventSource(true);
}

void GTestTCPInterface::TearDown()
{
  App()->getTCPServer()->setEventSource(false);
  m_reader->setEventSource(false);
}

TEST_F(GTestTCPInterface, StartInvalidateServer)
{
  EXPECT_NO_THROW(App()->getTCPServer()->bindAndListen());
  sleep(1);
  EXPECT_NO_THROW(App()->getTCPServer()->invalidate());
}

TEST_F(GTestTCPInterface, StartConnectReader)
{
  EXPECT_NO_THROW(App()->getTCPServer()->bindAndListen());
  sleep(1);
  EXPECT_EQ(m_reader->connect(), 0);
  EXPECT_NO_THROW(App()->getTCPServer()->invalidate());
}

TEST_F(GTestTCPInterface, RequestData_NoModules)
{
  EXPECT_NO_THROW(App()->getTCPServer()->bindAndListen());
  EXPECT_EQ(m_reader->connect(), 0);

  if (getuid() == 0) {
    App()->m_procAcct = std::make_shared<ProcAcct>(App()->getOptions());
    App()->getProcAcct()->setEventSource(true);

    App()->m_procRegistry = std::make_shared<ProcRegistry>(App()->getOptions());
    App()->getProcRegistry()->setEventSource(true);

    App()->m_procEvent = std::make_shared<ProcEvent>(App()->getOptions());
    App()->getProcEvent()->setEventSource(true);

    // Generate some data
    system("uname -a");

    EXPECT_EQ(m_reader->getProcAcctCount(), 0);
    EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcAcct), true);
    usleep(100000);
    EXPECT_GE(m_reader->getProcAcctCount(), 0);

    EXPECT_EQ(m_reader->getProcInfoCount(), 0);
    EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcInfo), true);
    usleep(100000);
    EXPECT_EQ(m_reader->getProcInfoCount(), 1);

    EXPECT_EQ(m_reader->getProcEventCount(), 0);
    EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcEventStats), true);
    usleep(100000);
    EXPECT_EQ(m_reader->getProcEventCount(), 1);
  }

  EXPECT_EQ(m_reader->getCtxInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetContextInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getCtxInfoCount(), 1);

  EXPECT_EQ(m_reader->getSysProcStatCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcStat), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcStatCount(), 0);

  EXPECT_EQ(m_reader->getSysProcMemInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcMemInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcMemInfoCount(), 0);

  EXPECT_EQ(m_reader->getSysProcDiskStatsCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcDiskStats), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcDiskStatsCount(), 0);

  EXPECT_EQ(m_reader->getSysProcPressureCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcPressure), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcPressureCount(), 0);

  EXPECT_EQ(m_reader->getSysProcBuddyInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcBuddyInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcBuddyInfoCount(), 0);

  EXPECT_EQ(m_reader->getSysProcWirelessCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcWireless), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcWirelessCount(), 0);

  if (getuid() == 0) {
    App()->getProcAcct()->setEventSource(false);
    App()->getProcRegistry()->setEventSource(false);
    App()->getProcEvent()->setEventSource(false);
  }
}

TEST_F(GTestTCPInterface, RequestData)
{
  EXPECT_NO_THROW(App()->getTCPServer()->bindAndListen());
  sleep(1);
  EXPECT_EQ(m_reader->connect(), 0);

  App()->m_procAcct = std::make_shared<ProcAcct>(App()->getOptions());
  App()->getProcAcct()->setEventSource(true);

  App()->m_procRegistry = std::make_shared<ProcRegistry>(App()->getOptions());
  App()->getProcRegistry()->setEventSource(true);

  App()->m_procEvent = std::make_shared<ProcEvent>(App()->getOptions());
  App()->getProcEvent()->setEventSource(true);

  App()->m_sysProcStat = std::make_shared<SysProcStat>(App()->getOptions());
  App()->getSysProcStat()->setEventSource(true);

  App()->m_sysProcMemInfo = std::make_shared<SysProcMemInfo>(App()->getOptions());
  App()->getSysProcMemInfo()->setEventSource(true);

  App()->m_sysProcDiskStats = std::make_shared<SysProcDiskStats>(App()->getOptions());
  App()->getSysProcDiskStats()->setEventSource(true);

  App()->m_sysProcPressure = std::make_shared<SysProcPressure>(App()->getOptions());
  App()->getSysProcPressure()->setEventSource(true);

  App()->m_sysProcBuddyInfo = std::make_shared<SysProcBuddyInfo>(App()->getOptions());
  App()->getSysProcBuddyInfo()->setEventSource(true);

  App()->m_sysProcWireless = std::make_shared<SysProcWireless>(App()->getOptions());
  App()->getSysProcWireless()->setEventSource(true);

  // Generate some data
  system("uname -a");
  App()->getProcRegistry()->update();
  App()->getSysProcStat()->update();
  App()->getSysProcMemInfo()->update();
  App()->getSysProcDiskStats()->update();
  App()->getSysProcPressure()->update();
  App()->getSysProcBuddyInfo()->update();
  App()->getSysProcWireless()->update();
  sleep(1);

  EXPECT_EQ(m_reader->getProcAcctCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcAcct), true);
  usleep(100000);
  EXPECT_GE(m_reader->getProcAcctCount(), 0);

  EXPECT_EQ(m_reader->getProcInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getProcInfoCount(), 1);

  EXPECT_EQ(m_reader->getProcEventCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcEventStats), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getProcEventCount(), 1);

  EXPECT_EQ(m_reader->getCtxInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetContextInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getCtxInfoCount(), 1);

  EXPECT_EQ(m_reader->getSysProcStatCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcStat), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcStatCount(), 1);

  EXPECT_EQ(m_reader->getSysProcMemInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcMemInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcMemInfoCount(), 1);

  EXPECT_EQ(m_reader->getSysProcDiskStatsCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcDiskStats), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcDiskStatsCount(), 1);

  EXPECT_EQ(m_reader->getSysProcPressureCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcPressure), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcPressureCount(), 1);

  EXPECT_EQ(m_reader->getSysProcBuddyInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcBuddyInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getSysProcBuddyInfoCount(), 1);

  EXPECT_EQ(m_reader->getSysProcWirelessCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetSysProcWireless), true);
  usleep(100000);
  EXPECT_GE(m_reader->getSysProcWirelessCount(), 0);

  App()->getProcAcct()->setEventSource(false);
  App()->getProcRegistry()->setEventSource(false);
  App()->getProcEvent()->setEventSource(false);
  App()->getSysProcStat()->setEventSource(false);
  App()->getSysProcMemInfo()->setEventSource(false);
  App()->getSysProcDiskStats()->setEventSource(false);
  App()->getSysProcPressure()->setEventSource(false);
  App()->getSysProcBuddyInfo()->setEventSource(false);
  App()->getSysProcWireless()->setEventSource(false);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
