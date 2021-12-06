/*
 * SPDX license identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2021 Alin Popa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * \author Alin Popa <alin.popa@fxdata.ro>
 */

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "../source/Application.h"
#include "gtest/gtest.h"

using namespace std;
using namespace tkm::monitor;

class GTestApplication : public ::testing::Test
{
protected:
    GTestApplication();
    virtual ~GTestApplication();

    virtual void SetUp();
    virtual void TearDown();

protected:
    unique_ptr<Application> m_app;
};

GTestApplication::GTestApplication()
{
    m_app = make_unique<Application>(
        "TKM", "TaskMonitor Application", tkmDefaults.getFor(Defaults::Default::ConfPath));
}

GTestApplication::~GTestApplication() { }

void GTestApplication::SetUp() { }

void GTestApplication::TearDown() { }

TEST_F(GTestApplication, parse)
{
    if (TaskMonitor()->hasConfigFile()) {
        TaskMonitor()->getConfigFile()->printStdOutput();
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
