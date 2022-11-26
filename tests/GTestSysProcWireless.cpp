/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcWireless Class Unit Tets
 * @details   GTests for SysProcWireless class
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

class GTestSysProcWireless : public ::testing::Test
{
protected:
  GTestSysProcWireless();
  virtual ~GTestSysProcWireless();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
  std::shared_ptr<Collector> m_collector = nullptr;
  std::shared_ptr<Client> m_client = nullptr;
  int m_sockets[2];
};

GTestSysProcWireless::GTestSysProcWireless()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  m_ownThread = std::make_unique<std::thread>(appRun);
}

GTestSysProcWireless::~GTestSysProcWireless()
{
  App()->stop();
  m_ownThread->join();
  app.reset();
}

void GTestSysProcWireless::SetUp()
{
  EXPECT_NE(socketpair(AF_UNIX, SOCK_STREAM, 0, m_sockets), -1);
  m_collector = std::make_shared<Collector>(m_sockets[0]);
  m_collector->setEventSource(true);
  m_client = std::make_shared<Client>(m_sockets[1]);
  m_client->setEventSource(true);

  App()->m_sysProcWireless = std::make_shared<SysProcWireless>(App()->getOptions());
  App()->getSysProcWireless()->setEventSource(true);
}

void GTestSysProcWireless::TearDown()
{
  App()->getSysProcWireless()->setEventSource(false);
  m_collector->setEventSource(false);
  m_client->setEventSource(false);
}

TEST_F(GTestSysProcWireless, UpdateWireless)
{
  App()->getSysProcWireless()->update();
  sleep(1);
}

TEST_F(GTestSysProcWireless, RequestData)
{
  App()->getSysProcWireless()->update();
  sleep(1);

  SysProcWireless::Request rq = {.action = SysProcWireless::Action::CollectAndSend,
                                 .collector = m_collector};
  App()->getSysProcWireless()->pushRequest(rq);
  sleep(1);

  // Unpack message from envelope
  tkm::msg::monitor::Message msg;
  m_client->getLastEnvelope().mesg().UnpackTo(&msg);
  EXPECT_EQ(msg.type(), tkm::msg::monitor::Message_Type_Data);

  // Unpack data from message
  tkm::msg::monitor::Data data;
  msg.payload().UnpackTo(&data);
  EXPECT_EQ(data.what(), tkm::msg::monitor::Data_What_SysProcWireless);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
