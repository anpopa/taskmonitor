/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Alin Popa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * @author Alin Popa <alin.popa@fxdata.ro>
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
