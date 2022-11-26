/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcStat Class Unit Tets
 * @details   GTests for SysProcStat class
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

class GTestSysProcStat : public ::testing::Test
{
protected:
  GTestSysProcStat();
  virtual ~GTestSysProcStat();

  virtual void SetUp();
  virtual void TearDown();

protected:
  std::unique_ptr<std::thread> m_ownThread = nullptr;
  std::shared_ptr<Collector> m_collector = nullptr;
  std::shared_ptr<Client> m_client = nullptr;
  int m_sockets[2];
};

GTestSysProcStat::GTestSysProcStat()
{
  app = std::make_unique<Application>("TKM", "TaskMonitor Application", "assets/taskmonitor.conf");
  m_ownThread = std::make_unique<std::thread>(appRun);
}

GTestSysProcStat::~GTestSysProcStat()
{
  App()->stop();
  m_ownThread->join();
  app.reset();
}

void GTestSysProcStat::SetUp()
{
  EXPECT_NE(socketpair(AF_UNIX, SOCK_STREAM, 0, m_sockets), -1);
  m_collector = std::make_shared<Collector>(m_sockets[0]);
  m_collector->setEventSource(true);
  m_client = std::make_shared<Client>(m_sockets[1]);
  m_client->setEventSource(true);

  App()->m_sysProcStat = std::make_shared<SysProcStat>(App()->getOptions());
  App()->getSysProcStat()->setEventSource(true);
}

void GTestSysProcStat::TearDown()
{
  App()->getSysProcStat()->setEventSource(false);
  m_collector->setEventSource(false);
  m_client->setEventSource(false);
}

TEST_F(GTestSysProcStat, UpdateCPUStat)
{
  EXPECT_EQ(App()->getSysProcStat()->getCPUStat("cpu"), nullptr);

  App()->getSysProcStat()->update();
  sleep(1);

  const std::shared_ptr<CPUStat> cpu = App()->getSysProcStat()->getCPUStat("cpu");
  EXPECT_NE(cpu, nullptr);
}

TEST_F(GTestSysProcStat, RequestData)
{
  EXPECT_EQ(App()->getSysProcStat()->getCPUStat("cpu"), nullptr);

  App()->getSysProcStat()->update();
  sleep(1);

  // Do a second update to get the diffs
  App()->getSysProcStat()->update();
  sleep(1);

  const std::shared_ptr<CPUStat> cpu = App()->getSysProcStat()->getCPUStat("cpu");
  EXPECT_NE(cpu, nullptr);

  SysProcStat::Request rq = {.action = SysProcStat::Action::CollectAndSend,
                             .collector = m_collector};
  App()->getSysProcStat()->pushRequest(rq);
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
