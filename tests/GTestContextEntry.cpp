/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ContextEntry Class Unit Tets
 * @details   GTests for ContextEntry class
 *-
 */

#include <fstream>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "../source/ContextEntry.h"

using namespace tkm::monitor;

class GTestContextEntry : public ::testing::Test
{
protected:
  GTestContextEntry() = default;
  virtual ~GTestContextEntry();
};

GTestContextEntry::~GTestContextEntry() {}

TEST_F(GTestContextEntry, CreateEntry)
{
  std::shared_ptr<ContextEntry> entry = nullptr;
  entry = std::make_shared<ContextEntry>(0xABABABAB, "TestEntry");

  EXPECT_EQ(entry->getContextId(), 0xABABABAB);
  EXPECT_STRCASEEQ(entry->getData().ctx_name().c_str(), "TestEntry");

  entry->resetData();
}

TEST_F(GTestContextEntry, ResetData)
{
  std::shared_ptr<ContextEntry> entry = nullptr;
  entry = std::make_shared<ContextEntry>(0xABABABAB, "TestEntry");

  tkm::msg::monitor::ContextInfoEntry data;

  data.set_total_cpu_time(10);
  entry->setData(data);

  EXPECT_EQ(entry->getData().total_cpu_time(), 10);
  entry->resetData();
  EXPECT_EQ(entry->getData().total_cpu_time(), 0);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
