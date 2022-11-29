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

class GTestUDSInterface : public ::testing::Test
{
protected:
  GTestUDSInterface();
  virtual ~GTestUDSInterface();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
  std::shared_ptr<Reader> m_reader = nullptr;
};

GTestUDSInterface::GTestUDSInterface()
{
  app = std::make_unique<Application>(
      "TKM", "TaskMonitor Application", "assets/taskmonitor_var0.conf");
  m_ownThread = std::make_unique<std::thread>(appRun);
}

GTestUDSInterface::~GTestUDSInterface()
{
  App()->stop();
  m_ownThread->join();
  app.reset();
}

void GTestUDSInterface::SetUp()
{
  App()->m_udsServer = std::make_shared<UDSServer>(App()->getOptions());
  m_reader = std::make_shared<Reader>(App()->getOptions(), Reader::Type::UNIX);
  m_reader->setEventSource(true);
}

void GTestUDSInterface::TearDown()
{
  App()->getUDSServer()->setEventSource(false);
  m_reader->setEventSource(false);
}

TEST_F(GTestUDSInterface, StartInvalidateServer)
{
  EXPECT_NO_THROW(App()->getUDSServer()->start());
  sleep(1);
  EXPECT_NO_THROW(App()->getUDSServer()->stop());
}

TEST_F(GTestUDSInterface, StartConnectReader)
{
  EXPECT_NO_THROW(App()->getUDSServer()->start());
  sleep(1);
  EXPECT_EQ(m_reader->connect(), 0);
  EXPECT_NO_THROW(App()->getUDSServer()->stop());
}

TEST_F(GTestUDSInterface, RequestData_NoModules)
{
  EXPECT_NO_THROW(App()->getUDSServer()->start());
  sleep(1);
  EXPECT_EQ(m_reader->connect(), 0);

#ifdef WITH_PROC_ACCT
  App()->m_procAcct = std::make_shared<ProcAcct>(App()->getOptions());
  App()->getProcAcct()->setEventSource(true);
#endif
#ifdef WITH_PROC_EVENT
  if (getuid() == 0) {
    App()->m_procEvent = std::make_shared<ProcEvent>(App()->getOptions());
    App()->getProcEvent()->setEventSource(true);
  }
#endif
  App()->m_procRegistry = std::make_shared<ProcRegistry>(App()->getOptions());
  App()->getProcRegistry()->setEventSource(true);

  // Generate some data
#ifdef WITH_PROC_EVENT
  if (getuid() == 0) {
    system("sleep 3");
  }
#else
  App()->getProcRegistry()->update();
  sleep(3);
#endif

#ifdef WITH_PROC_ACCT
  EXPECT_EQ(m_reader->getProcAcctCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcAcct), true);
  usleep(100000);
  EXPECT_GE(m_reader->getProcAcctCount(), 0);
#endif
#ifdef WITH_PROC_EVENT
  if (getuid() == 0) {
    EXPECT_EQ(m_reader->getProcEventCount(), 0);
    EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcEventStats), true);
    usleep(100000);
    EXPECT_EQ(m_reader->getProcEventCount(), 1);
  }
#endif

  EXPECT_EQ(m_reader->getProcInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getProcInfoCount(), 1);

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

#ifdef WITH_PROC_ACCT
  App()->getProcAcct()->setEventSource(false);
#endif
  App()->getProcRegistry()->setEventSource(false);
#ifdef WITH_PROC_EVENT
  if (getuid() == 0) {
    App()->getProcEvent()->setEventSource(false);
  }
#endif
}

TEST_F(GTestUDSInterface, RequestData)
{
  EXPECT_NO_THROW(App()->getUDSServer()->start());
  sleep(1);
  EXPECT_EQ(m_reader->connect(), 0);

#ifdef WITH_PROC_ACCT
  App()->m_procAcct = std::make_shared<ProcAcct>(App()->getOptions());
  App()->getProcAcct()->setEventSource(true);
#endif

  App()->m_procRegistry = std::make_shared<ProcRegistry>(App()->getOptions());
  App()->getProcRegistry()->setEventSource(true);
#ifdef WITH_PROC_EVENT
  if (getuid() == 0) {
    App()->m_procEvent = std::make_shared<ProcEvent>(App()->getOptions());
    App()->getProcEvent()->setEventSource(true);
  }
#endif
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
  App()->getSysProcStat()->update();
  App()->getSysProcMemInfo()->update();
  App()->getSysProcDiskStats()->update();
  App()->getSysProcPressure()->update();
  App()->getSysProcBuddyInfo()->update();
  App()->getSysProcWireless()->update();

#ifdef WITH_PROC_EVENT
  if (getuid() == 0) {
    system("sleep 3");
  }
#else
  App()->getProcRegistry()->update();
  sleep(3);
#endif

#ifdef WITH_PROC_ACCT
  EXPECT_EQ(m_reader->getProcAcctCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcAcct), true);
  usleep(100000);
  EXPECT_GE(m_reader->getProcAcctCount(), 0);
#endif
  EXPECT_EQ(m_reader->getProcInfoCount(), 0);
  EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcInfo), true);
  usleep(100000);
  EXPECT_EQ(m_reader->getProcInfoCount(), 1);
#ifdef WITH_PROC_EVENT
  if (getuid() == 0) {
    EXPECT_EQ(m_reader->getProcEventCount(), 0);
    EXPECT_EQ(m_reader->requestData(tkm::msg::collector::Request_Type_GetProcEventStats), true);
    usleep(100000);
    EXPECT_EQ(m_reader->getProcEventCount(), 1);
  }
#endif

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

#ifdef WITH_PROC_ACCT
  App()->getProcAcct()->setEventSource(false);
#endif
  App()->getProcRegistry()->setEventSource(false);
#ifdef WITH_PROC_EVENT
  if (getuid() == 0) {
    App()->getProcEvent()->setEventSource(false);
  }
#endif
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
