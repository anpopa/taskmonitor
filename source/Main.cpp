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

#include "Logger.h"
#include <csignal>
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include <taskmonitor/taskmonitor.h>

#include "Application.h"

using namespace tkm::monitor;

std::unique_ptr<tkm::monitor::Application> app = nullptr;

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
    default:
      help = true;
      break;
    }
  }

  if (help) {
    std::cout << "TaskMonitor: monitor system resources\n"
              << "Version: " << tkmDefaults.getFor(Defaults::Default::Version)
              << " libtkm: " << TKMLIB_VERSION << "\n\n";
    std::cout << "Usage: taskmonitor [OPTIONS] \n\n";
    std::cout << "  General:\n";
    std::cout << "     --config, -c <string> Configuration file path\n";
    std::cout << "  Help:\n";
    std::cout << "     --help, -h             Print this help\n\n";

    ::exit(EXIT_SUCCESS);
  }

  ::signal(SIGPIPE, SIG_IGN);
  ::signal(SIGINT, terminate);
  ::signal(SIGTERM, terminate);

  try {
    TKMLIB_CHECK_VERSION;
  } catch (...) {
    std::cout << "The libtaskmonitor headers mismatch library" << std::endl;
    ::exit(EXIT_FAILURE);
  }

  fs::path configPath(tkmDefaults.getFor(Defaults::Default::ConfPath));
  if (config_path != nullptr) {
    if (!fs::exists(config_path)) {
      std::cout << "Provided configuration file cannot be accesed: " << config_path << std::endl;
      return EXIT_FAILURE;
    }
    configPath = std::string(config_path);
  }

  app = std::make_unique<tkm::monitor::Application>("TaskMonitor", "TaskMonitor", configPath);
  app->run();

  return EXIT_SUCCESS;
}
