/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcMemInfo Class Unit Tets
 * @details   GTests for SysProcMemInfo class
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

class GTestSysProcMemInfo : public ::testing::Test
{
protected:
  GTestSysProcMemInfo();
  virtual ~GTestSysProcMemInfo();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
  std::shared_ptr<Collector> m_collector = nullptr;
  std::shared_ptr<Client> m_client = nullptr;
  int m_sockets[2];
};

GTestSysProcMemInfo::GTestSysProcMemInfo()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  m_ownThread = std::make_unique<std::thread>(appRun);
}

GTestSysProcMemInfo::~GTestSysProcMemInfo()
{
  App()->stop();
  m_ownThread->join();
  app.reset();
}

void GTestSysProcMemInfo::SetUp()
{
  EXPECT_NE(socketpair(AF_UNIX, SOCK_STREAM, 0, m_sockets), -1);
  m_collector = std::make_shared<Collector>(m_sockets[0]);
  m_collector->setEventSource(true);
  m_client = std::make_shared<Client>(m_sockets[1]);
  m_client->setEventSource(true);

  App()->m_sysProcMemInfo = std::make_shared<SysProcMemInfo>(App()->getOptions());
  App()->getSysProcMemInfo()->setEventSource(true);
}

void GTestSysProcMemInfo::TearDown()
{
  App()->getSysProcMemInfo()->setEventSource(false);
  m_collector->setEventSource(false);
  m_client->setEventSource(false);
}

TEST_F(GTestSysProcMemInfo, UpdateMemInfo)
{
  App()->getSysProcMemInfo()->update();
  sleep(1);
}

TEST_F(GTestSysProcMemInfo, RequestData)
{
  App()->getSysProcMemInfo()->update();
  sleep(1);

  SysProcMemInfo::Request rq = {.action = SysProcMemInfo::Action::CollectAndSend,
                                .collector = m_collector};
  App()->getSysProcMemInfo()->pushRequest(rq);
  sleep(1);

  // Unpack message from envelope
  tkm::msg::monitor::Message msg;
  m_client->getLastEnvelope().mesg().UnpackTo(&msg);
  EXPECT_EQ(msg.type(), tkm::msg::monitor::Message_Type_Data);

  // Unpack data from message
  tkm::msg::monitor::Data data;
  msg.payload().UnpackTo(&data);
  EXPECT_EQ(data.what(), tkm::msg::monitor::Data_What_SysProcMemInfo);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
