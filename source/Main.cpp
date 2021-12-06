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

#include "Application.h"
#include "Defaults.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <memory.h>

using namespace std;
using namespace tkm::monitor;
namespace fs = std::filesystem;

static void terminate(int signum)
{
    logInfo() << "Received signal " << signum;
    exit(EXIT_SUCCESS);
}

auto main(int argc, char **argv) -> int
{
    const char *config_path = nullptr;
    int long_index = 0;
    bool help = false;
    int c;

    struct option longopts[] = {{"config", required_argument, nullptr, 'c'},
                                {"help", no_argument, nullptr, 'h'},
                                {nullptr, 0, nullptr, 0}};

    while ((c = getopt_long(argc, argv, "c:h", longopts, &long_index)) != -1) {
        switch (c) {
        case 'c':
            config_path = optarg;
            break;
        case 'h':
            help = true;
            break;
        default:
            break;
        }
    }

    if (help) {
        cout << "TaskMonitor: Monitor system resources"
             << tkmDefaults.getFor(Defaults::Default::Version) << "\n\n";
        cout << "Usage: taskmonitor [OPTIONS] \n\n";
        cout << "  General:\n";
        cout << "     --config, -c      <string> Configuration file path\n";
        cout << "  Help:\n";
        cout << "     --help, -h                 Print this help\n\n";

        exit(EXIT_SUCCESS);
    }

    signal(SIGINT, terminate);
    signal(SIGTERM, terminate);

    fs::path configPath(tkmDefaults.getFor(Defaults::Default::ConfPath));
    if (config_path != nullptr) {
        if (!fs::exists(config_path)) {
            cout << "Provided configuration file cannot be accesed: " << config_path << endl;
            return EXIT_FAILURE;
        }
        configPath = string(config_path);
    }

    Application app {"TaskMonitor", "TaskMonitor", configPath};

    // Request connection
    ActionManager::Request registerEvents {
        .action = ActionManager::Action::RegisterEvents,
    };
    app.getManager()->pushRequest(registerEvents);

    app.run();

    return EXIT_SUCCESS;
}
