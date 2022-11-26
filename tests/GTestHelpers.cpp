/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Helpers Class Unit Tets
 * @details   GTests for Helpers class
 *-
 */

#include <fstream>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taskmonitor/taskmonitor.h>
#include <utility>

#include "../source/Helpers.h"

class GTestHelpers : public ::testing::Test
{
protected:
  GTestHelpers() = default;
  virtual ~GTestHelpers();
};

GTestHelpers::~GTestHelpers() {}

TEST_F(GTestHelpers, GetContextName_Unknown)
{
  // Need to be root to access PID 1 cgroup NS
  if (getuid() == 0) {
    std::string ctxName = tkm::getContextName(std::string(), 0x01);
    EXPECT_STRCASEEQ(ctxName.c_str(), "unknown");
  }
}

TEST_F(GTestHelpers, GetContextName_Root)
{
  // Need to be root to access PID 1 cgroup NS
  if (getuid() == 0) {
    uint64_t rooId = tkm::getContextId(1);
    std::string ctxName = tkm::getContextName(std::string(), rooId);
    EXPECT_STRCASEEQ(ctxName.c_str(), "root");
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
