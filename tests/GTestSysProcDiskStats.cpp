/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcDiskStats Class Unit Tets
 * @details   GTests for SysProcDiskStats class
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

class GTestSysProcDiskStats : public ::testing::Test
{
protected:
  GTestSysProcDiskStats();
  virtual ~GTestSysProcDiskStats();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
  std::shared_ptr<Collector> m_collector = nullptr;
  std::shared_ptr<Client> m_client = nullptr;
  int m_sockets[2];
};

GTestSysProcDiskStats::GTestSysProcDiskStats()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  m_ownThread = std::make_unique<std::thread>(appRun);
}

GTestSysProcDiskStats::~GTestSysProcDiskStats()
{
  App()->stop();
  m_ownThread->join();
  app.reset();
}

void GTestSysProcDiskStats::SetUp()
{
  EXPECT_NE(socketpair(AF_UNIX, SOCK_STREAM, 0, m_sockets), -1);
  m_collector = std::make_shared<Collector>(m_sockets[0]);
  m_collector->setEventSource(true);
  m_client = std::make_shared<Client>(m_sockets[1]);
  m_client->setEventSource(true);

  App()->m_sysProcDiskStats = std::make_shared<SysProcDiskStats>(App()->getOptions());
  App()->getSysProcDiskStats()->setEventSource(true);
}

void GTestSysProcDiskStats::TearDown()
{
  App()->getSysProcDiskStats()->setEventSource(false);
  m_collector->setEventSource(false);
  m_client->setEventSource(false);
}

TEST_F(GTestSysProcDiskStats, UpdateDiskStats)
{
  App()->getSysProcDiskStats()->update();
  sleep(1);
}

TEST_F(GTestSysProcDiskStats, RequestData)
{
  App()->getSysProcDiskStats()->update();
  sleep(1);

  SysProcDiskStats::Request rq = {.action = SysProcDiskStats::Action::CollectAndSend,
                                  .collector = m_collector};
  App()->getSysProcDiskStats()->pushRequest(rq);
  sleep(1);

  // Unpack message from envelope
  tkm::msg::monitor::Message msg;
  m_client->getLastEnvelope().mesg().UnpackTo(&msg);
  EXPECT_EQ(msg.type(), tkm::msg::monitor::Message_Type_Data);

  // Unpack data from message
  tkm::msg::monitor::Data data;
  msg.payload().UnpackTo(&data);
  EXPECT_EQ(data.what(), tkm::msg::monitor::Data_What_SysProcDiskStats);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
