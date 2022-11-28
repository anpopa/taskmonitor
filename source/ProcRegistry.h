/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     ProcRegistry Class
 * @details   ProcEntry registry
 *-
 */

#pragma once

#include <string>

#include "ContextEntry.h"
#include "ICollector.h"
#include "Options.h"
#include "ProcEntry.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{

class ProcRegistry : public IDataSource, public std::enable_shared_from_this<ProcRegistry>
{
public:
  enum class Action {
    CommitProcList,
    CommitContextList,
    CollectAndSendProcAcct,
    CollectAndSendProcInfo,
    CollectAndSendContextInfo
  };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit ProcRegistry(const std::shared_ptr<Options> options);
  virtual ~ProcRegistry() = default;

public:
  ProcRegistry(ProcRegistry const &) = delete;
  void operator=(ProcRegistry const &) = delete;

public:
  auto getShared() -> std::shared_ptr<ProcRegistry> { return shared_from_this(); }
  void setEventSource(bool enabled = true);
  void initFromProc(void);

  void addProcEntry(int pid);
  void updProcEntry(int pid);
  void remProcEntry(int pid, bool sync = false);
  void remProcEntry(const std::string &name, bool sync = false);
  auto getProcEntry(int pid) -> const std::shared_ptr<ProcEntry>;
  auto getProcEntry(const std::string &name) -> const std::shared_ptr<ProcEntry>;
  auto getProcList(void) -> bswi::util::SafeList<std::shared_ptr<ProcEntry>> &
  {
    return m_procList;
  }
  auto getContextList(void) -> bswi::util::SafeList<std::shared_ptr<ContextEntry>> &
  {
    return m_contextList;
  }

  auto pushRequest(ProcRegistry::Request &request) -> int;
  bool update(UpdateLane lane) final;
  bool update(void) final;

private:
  bool requestHandler(const Request &request);
  auto getProcNameForPID(int pid) -> std::string;
  bool isBlacklisted(const std::string &name);
  void createProcessEntry(int pid, const std::string &name);
  void updateProcessList(void);

private:
  bswi::util::SafeList<std::shared_ptr<ContextEntry>> m_contextList{"ProcRegistryContextList"};
  bswi::util::SafeList<std::shared_ptr<ProcEntry>> m_procList{"ProcRegistryProcList"};
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
};

} // namespace tkm::monitor
