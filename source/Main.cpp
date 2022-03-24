/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Main function
 * @details   Main function and signal handlers
 *-
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

    ActionManager::Request registerEvents {
        .action = ActionManager::Action::RegisterEvents,
    };
    app.getManager()->pushRequest(registerEvents);

    app.run();

    return EXIT_SUCCESS;
}
