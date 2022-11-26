/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     StartupData Class Unit Tets
 * @details   GTests for StartupData class
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
#include "../tests/dummy/Client.h"
#include "../tests/dummy/Collector.h"

using namespace tkm::monitor;

static std::unique_ptr<Application> app = nullptr;

static void appRun()
{
  App()->run();
}

class GTestStartupData : public ::testing::Test
{
protected:
  GTestStartupData();
  virtual ~GTestStartupData();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
  std::shared_ptr<Collector> m_collector = nullptr;
  std::shared_ptr<Client> m_client = nullptr;
  int m_sockets[2];
};

GTestStartupData::GTestStartupData()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  m_ownThread = std::make_unique<std::thread>(appRun);
}

GTestStartupData::~GTestStartupData()
{
  App()->stop();
  m_ownThread->join();
  app.reset();
}

void GTestStartupData::SetUp()
{
  EXPECT_NE(socketpair(AF_UNIX, SOCK_STREAM, 0, m_sockets), -1);
  m_collector = std::make_shared<Collector>(m_sockets[0]);
  m_collector->setEventSource(true);
  m_client = std::make_shared<Client>(m_sockets[1]);
  m_client->setEventSource(true);

  App()->m_startupData = std::make_shared<StartupData>(App()->getOptions());
  App()->getStartupData()->setEventSource(true);
}

void GTestStartupData::TearDown()
{
  App()->getStartupData()->setEventSource(false);
  m_collector->setEventSource(false);
  m_client->setEventSource(false);
}

TEST_F(GTestStartupData, DropData)
{
  EXPECT_EQ(App()->getStartupData()->getCpuList().size(), 0);
  EXPECT_EQ(App()->getStartupData()->getMemList().size(), 0);
  EXPECT_EQ(App()->getStartupData()->getPsiList().size(), 0);

  tkm::msg::monitor::SysProcStat statData{};
  tkm::msg::monitor::SysProcMemInfo memData{};
  tkm::msg::monitor::SysProcPressure psiData{};

  App()->getStartupData()->addCpuData(statData);
  App()->getStartupData()->addMemData(memData);
  App()->getStartupData()->addPsiData(psiData);

  EXPECT_EQ(App()->getStartupData()->getCpuList().size(), 1);
  EXPECT_EQ(App()->getStartupData()->getMemList().size(), 1);
  EXPECT_EQ(App()->getStartupData()->getPsiList().size(), 1);

  EXPECT_EQ(App()->getStartupData()->expired(), false);
  App()->getStartupData()->dropData();
  EXPECT_EQ(App()->getStartupData()->expired(), true);

  EXPECT_EQ(App()->getStartupData()->getCpuList().size(), 0);
  EXPECT_EQ(App()->getStartupData()->getMemList().size(), 0);
  EXPECT_EQ(App()->getStartupData()->getPsiList().size(), 0);
}

TEST_F(GTestStartupData, RequestData)
{
  tkm::msg::monitor::SysProcStat statData{};
  tkm::msg::monitor::SysProcMemInfo memData{};
  tkm::msg::monitor::SysProcPressure psiData{};

  App()->getStartupData()->addCpuData(statData);
  App()->getStartupData()->addMemData(memData);
  App()->getStartupData()->addPsiData(psiData);

  StartupData::Request rq = {.action = StartupData::Action::CollectAndSend,
                             .collector = m_collector};
  App()->getStartupData()->pushRequest(rq);
  sleep(1);

  // The PSI data is sent last so last message should be psiData

  // Unpack message from envelope
  tkm::msg::monitor::Message msg;
  m_client->getLastEnvelope().mesg().UnpackTo(&msg);
  EXPECT_EQ(msg.type(), tkm::msg::monitor::Message_Type_Data);

  // Unpack data from message
  tkm::msg::monitor::Data data;
  msg.payload().UnpackTo(&data);
  EXPECT_EQ(data.what(), tkm::msg::monitor::Data_What_SysProcPressure);
}

TEST_F(GTestStartupData, RequestData_OneMessage)
{
  tkm::msg::monitor::SysProcStat statData{};
  App()->getStartupData()->addCpuData(statData);

  StartupData::Request rq = {.action = StartupData::Action::CollectAndSend,
                             .collector = m_collector};
  App()->getStartupData()->pushRequest(rq);
  sleep(1);

  // Unpack message from envelope
  tkm::msg::monitor::Message msg;
  m_client->getLastEnvelope().mesg().UnpackTo(&msg);
  EXPECT_EQ(msg.type(), tkm::msg::monitor::Message_Type_Data);

  // Unpack data from message
  tkm::msg::monitor::Data data;
  msg.payload().UnpackTo(&data);
  EXPECT_EQ(data.what(), tkm::msg::monitor::Data_What_SysProcStat);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
