/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Registry Class
 * @details   ProcEntry registry
 *-
 */

#include "Registry.h"
#include "Application.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>

#include "../bswinfra/source/KeyFile.h"

namespace fs = std::filesystem;
using namespace bswi::kf;

namespace tkm::monitor
{

static bool doCollectAndSend(const std::shared_ptr<Registry> &mgr,
                             const Registry::Request &request);

Registry::Registry(std::shared_ptr<Options> &options)
: m_options(options)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "RegistryEventQueue", [this](const Request &request) { return requestHandler(request); });
}

auto Registry::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void Registry::enableEvents()
{
  App()->addEventSource(m_queue);
}

auto Registry::requestHandler(const Request &request) -> bool
{
  switch (request.action) {
  case Registry::Action::CollectAndSend:
    return doCollectAndSend(getShared(), request);
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

void Registry::initFromProc(void)
{
  std::string path = "/proc";

  logDebug() << "Read existing proc entries";
  for (const auto &entry : fs::directory_iterator(path)) {
    int pid = -1;

    try {
      pid = std::stoi(entry.path().filename());
    } catch (...) {
      // Discard non pid entries
    }

    if (pid != -1) {
      if (!isBlacklisted(pid)) {
        logDebug() << "Add process monitoring for pid " << pid;
        std::shared_ptr<ProcEntry> entry = std::make_shared<ProcEntry>(pid, getProcNameForPID(pid));
        m_list.append(entry);
      }
    }
  }

  // Commit our updated list
  m_list.commit();
}

auto Registry::getEntry(int pid) -> const std::shared_ptr<ProcEntry>
{
  std::shared_ptr<ProcEntry> retEntry = nullptr;

  m_list.foreach ([this, pid, &retEntry](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getPid() == pid) {
      retEntry = entry;
    }
  });

  return retEntry;
}

auto Registry::getEntry(std::string &name) -> const std::shared_ptr<ProcEntry>
{
  std::shared_ptr<ProcEntry> retEntry = nullptr;

  m_list.foreach ([this, &name, &retEntry](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getName() == name) {
      retEntry = entry;
    }
  });

  return retEntry;
}

void Registry::addEntry(int pid)
{
  auto found = false;

  m_list.foreach ([this, &found, pid](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getPid() == pid) {
      found = true;
    }
  });

  if (!found && !isBlacklisted(pid)) {
    std::shared_ptr<ProcEntry> entry = std::make_shared<ProcEntry>(pid, getProcNameForPID(pid));
    m_list.append(entry);
    m_list.commit();
  }
}

void Registry::remEntry(int pid)
{
  m_list.foreach ([this, pid](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getPid() == pid) {
      logDebug() << "Found entry to remove with pid " << pid;
      m_list.remove(entry);
    }
  });
  m_list.commit();
}

void Registry::remEntry(std::string &name)
{
  m_list.foreach ([this, &name](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getName() == name) {
      logDebug() << "Found entry to remove with pid " << entry->getPid();
      m_list.remove(entry);
    }
  });
  m_list.commit();
}

bool Registry::isBlacklisted(int pid)
{
  auto procName = getProcNameForPID(pid);

  if (m_options->hasConfigFile()) {
    const std::vector<Property> props = m_options->getConfigFile()->getProperties("blacklist", -1);
    for (const auto &prop : props) {
      if (procName.find(prop.key) != std::string::npos) {
        return true;
      }
    }
  }

  return false;
}

auto Registry::getProcNameForPID(int pid) -> std::string
{
  auto statusPath = fs::path("/proc") / fs::path(std::to_string(pid)) / fs::path("status");
  std::ifstream statusStream{statusPath};

  if (!statusStream.is_open()) {
    logError() << "Failed to open status file for pid " << pid;
    throw std::runtime_error("Fail to open /proc/<pid>/status file");
  }

  std::string line;
  while (std::getline(statusStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    if (line.find("Name") == std::string::npos) {
      break;
    }

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 3) {
      logError() << "Proc status file parse error";
      throw std::runtime_error("Fail to parse /proc/<pid>/status file");
    }

    return tokens[1];
  }

  logError() << "Failed to parse status file for pid " << pid;
  throw std::runtime_error("Fail to find the process name");
}

static bool doCollectAndSend(const std::shared_ptr<Registry> &mgr, const Registry::Request &request)
{
  mgr->getRegistryList().foreach ([&request](const std::shared_ptr<ProcEntry> &entry) {
    tkm::msg::monitor::Data data;

    data.set_what(tkm::msg::monitor::Data_What_ProcAcct);
    data.set_timestamp(time(NULL));

    data.mutable_payload()->PackFrom(entry->getAcct());
    request.collector->sendData(data);
  });
  return true;
}

} // namespace tkm::monitor
