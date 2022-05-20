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

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>

#include "../bswinfra/source/KeyFile.h"
#include "ContextEntry.h"
#include "Helpers.h"

namespace tkm::monitor
{

static bool doCommitProcList(const std::shared_ptr<Registry> mgr, const Registry::Request &rq);
static bool doCommitContextList(const std::shared_ptr<Registry> mgr, const Registry::Request &rq);
static bool doCollectAndSendProcAcct(const std::shared_ptr<Registry> mgr,
                                     const Registry::Request &rq);
static bool doCollectAndSendProcInfo(const std::shared_ptr<Registry> mgr,
                                     const Registry::Request &rq);
static bool doCollectAndSendContextInfo(const std::shared_ptr<Registry> mgr,
                                        const Registry::Request &rq);

Registry::Registry(const std::shared_ptr<Options> options)
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

  if (m_options->getFor(Options::Key::ReadProcAtInit) == "true") {
    initFromProc();
  }
}

auto Registry::requestHandler(const Request &request) -> bool
{
  switch (request.action) {
  case Registry::Action::CommitProcList:
    return doCommitProcList(getShared(), request);
  case Registry::Action::CommitContextList:
    return doCommitContextList(getShared(), request);
  case Registry::Action::CollectAndSendProcAcct:
    return doCollectAndSendProcAcct(getShared(), request);
  case Registry::Action::CollectAndSendProcInfo:
    return doCollectAndSendProcInfo(getShared(), request);
  case Registry::Action::CollectAndSendContextInfo:
    return doCollectAndSendContextInfo(getShared(), request);
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
  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    int pid = -1;

    try {
      pid = std::stoi(entry.path().filename());
    } catch (...) {
      continue;
    }

    if (pid != -1) {
      std::string procName;

      try {
        procName = getProcNameForPID(pid);
      } catch (...) {
        logError() << "Failed to read procname for pid=" << pid;
        continue;
      }

      if (!isBlacklisted(procName)) {
        createProcessEntry(pid, procName);
      }
    }
  }
}

auto Registry::getProcEntry(int pid) -> const std::shared_ptr<ProcEntry>
{
  std::shared_ptr<ProcEntry> retEntry = nullptr;

  m_procList.foreach ([this, pid, &retEntry](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getPid() == pid) {
      retEntry = entry;
    }
  });

  return retEntry;
}

auto Registry::getProcEntry(const std::string &name) -> const std::shared_ptr<ProcEntry>
{
  std::shared_ptr<ProcEntry> retEntry = nullptr;

  m_procList.foreach ([this, &name, &retEntry](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getName() == name) {
      retEntry = entry;
    }
  });

  return retEntry;
}

void Registry::addProcEntry(int pid)
{
  auto found = false;

  m_procList.foreach ([this, &found, pid](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getPid() == pid) {
      found = true;
    }
  });

  if (!found) {
    std::string procName;

    try {
      procName = getProcNameForPID(pid);
    } catch (...) {
      logWarn() << "Proc entry removed before entry added";
      return;
    }

    if (!isBlacklisted(procName)) {
      createProcessEntry(pid, procName);
    }
  }
}

void Registry::remProcEntry(int pid)
{
  m_procList.foreach ([this, pid](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getPid() == pid) {
      logDebug() << "Found entry to remove with pid " << pid;
      m_procList.remove(entry);
    }
  });

  Registry::Request rq = {.action = Registry::Action::CommitProcList};
  pushRequest(rq);
}

void Registry::remProcEntry(std::string &name)
{
  m_procList.foreach ([this, &name](const std::shared_ptr<ProcEntry> &entry) {
    if (entry->getName() == name) {
      logDebug() << "Found entry to remove with pid " << entry->getPid();
      m_procList.remove(entry);
    }
  });

  Registry::Request rq = {.action = Registry::Action::CommitProcList};
  pushRequest(rq);
}

bool Registry::isBlacklisted(const std::string &name)
{
  if (m_options->hasConfigFile()) {
    const std::vector<bswi::kf::Property> props =
        m_options->getConfigFile()->getProperties("blacklist", -1);
    for (const auto &prop : props) {
      if (name.find(prop.key) != std::string::npos) {
        return true;
      }
    }
  }

  return false;
}

auto Registry::getProcNameForPID(int pid) -> std::string
{
  auto statusPath = std::filesystem::path("/proc") / std::filesystem::path(std::to_string(pid)) /
                    std::filesystem::path("status");
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

    if (tokens.size() < 2) {
      logError() << "Proc status file parse error";
      throw std::runtime_error("Fail to parse /proc/<pid>/status file");
    }

    std::string name{tokens[1]};
    for (int i = 2; i < tokens.size(); i++) {
      name.append(" " + tokens[i]);
    }

    return name;
  }

  logError() << "Failed to parse status file for pid " << pid;
  throw std::runtime_error("Fail to find the process name");
}

void Registry::createProcessEntry(int pid, const std::string &name)
{
  std::shared_ptr<ProcEntry> procEntry = nullptr;
  uint64_t interval = 1000000;

  try {
    procEntry = std::make_shared<ProcEntry>(pid, name);
    interval = std::stoul(m_options->getFor(Options::Key::FastLaneInterval));
  } catch (std::exception &e) {
    logError() << "Cannot create ProcEntry object for pid=" << pid << " name=" << name
               << ". Reason=" << e.what();
    return;
  }

  // ProcInfo is on fast lane
  procEntry->setUpdateProcInfoInterval(interval);

  logDebug() << "Add process monitoring for pid=" << pid << " name=" << name
             << " context=" << procEntry->getInfo().ctx_name();
  m_procList.append(procEntry);

  Registry::Request rq = {.action = Registry::Action::CommitProcList};
  pushRequest(rq);

  bool found = false;
  m_contextList.foreach ([&found, &procEntry](const std::shared_ptr<ContextEntry> &ctxEntry) {
    if (procEntry->getContextId() == ctxEntry->getContextId()) {
      found = true;
    }
  });

  if (!found) {
    std::shared_ptr<ContextEntry> ctxEntry = std::make_shared<ContextEntry>(
        procEntry->getInfo().ctx_id(), procEntry->getInfo().ctx_name());
    m_contextList.append(ctxEntry);

    Registry::Request rq = {.action = Registry::Action::CommitContextList};
    pushRequest(rq);
  }
}

static bool doCommitProcList(const std::shared_ptr<Registry> mgr, const Registry::Request &rq)
{
  mgr->getProcList().commit();
  return true;
}

static bool doCommitContextList(const std::shared_ptr<Registry> mgr, const Registry::Request &rq)
{
  mgr->getContextList().commit();
  return true;
}

static bool doCollectAndSendProcAcct(const std::shared_ptr<Registry> mgr,
                                     const Registry::Request &rq)
{
  mgr->getProcList().foreach ([&rq](const std::shared_ptr<ProcEntry> &entry) {
    tkm::msg::monitor::Data data;

    data.set_what(tkm::msg::monitor::Data_What_ProcAcct);

    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    data.set_system_time_sec(currentTime.tv_sec);
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    data.set_monotonic_time_sec(currentTime.tv_sec);

    data.mutable_payload()->PackFrom(entry->getAcct());
    rq.collector->sendData(data);
  });

  return true;
}

static bool doCollectAndSendProcInfo(const std::shared_ptr<Registry> mgr,
                                     const Registry::Request &rq)
{
  mgr->getProcList().foreach ([&rq](const std::shared_ptr<ProcEntry> &entry) {
    tkm::msg::monitor::Data data;

    data.set_what(tkm::msg::monitor::Data_What_ProcInfo);

    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    data.set_system_time_sec(currentTime.tv_sec);
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    data.set_monotonic_time_sec(currentTime.tv_sec);

    data.mutable_payload()->PackFrom(entry->getInfo());
    rq.collector->sendData(data);
  });

  return true;
}

static bool doCollectAndSendContextInfo(const std::shared_ptr<Registry> mgr,
                                        const Registry::Request &rq)
{
  // Update Context data
  mgr->getContextList().foreach ([&mgr, &rq](const std::shared_ptr<ContextEntry> &ctxEntry) {
    // Reset data
    ctxEntry->resetData();

    bool found = false;
    mgr->getProcList().foreach ([&mgr, &ctxEntry, &found](
                                    const std::shared_ptr<ProcEntry> &procEntry) {
      if (ctxEntry->getContextId() == procEntry->getContextId()) {
        auto totalCPUTime = ctxEntry->getInfo().total_cpu_time() + procEntry->getInfo().cpu_time();
        ctxEntry->getInfo().set_total_cpu_time(totalCPUTime);
        auto totalCPUPercent =
            ctxEntry->getInfo().total_cpu_percent() + procEntry->getInfo().cpu_percent();
        ctxEntry->getInfo().set_total_cpu_percent(totalCPUPercent);
        auto totalMEMrss = ctxEntry->getInfo().total_mem_vmrss() + procEntry->getInfo().mem_vmrss();
        ctxEntry->getInfo().set_total_mem_vmrss(totalMEMrss);
        found = true;
      }
    });

    // If no process belongs to the context we remove the context
    if (!found) {
      mgr->getContextList().remove(ctxEntry);
    }
  });
  Registry::Request crq = {.action = Registry::Action::CommitContextList};
  mgr->pushRequest(crq);

  mgr->getContextList().foreach ([&rq](const std::shared_ptr<ContextEntry> &entry) {
    tkm::msg::monitor::Data data;

    data.set_what(tkm::msg::monitor::Data_What_ContextInfo);

    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    data.set_system_time_sec(currentTime.tv_sec);
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    data.set_monotonic_time_sec(currentTime.tv_sec);

    data.mutable_payload()->PackFrom(entry->getInfo());
    rq.collector->sendData(data);
  });

  return true;
}

} // namespace tkm::monitor
