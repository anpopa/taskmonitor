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

#pragma once

#include <list>
#include <memory>
#include <string>

#include "ICollector.h"
#include "Options.h"
#include "ProcEntry.h"

#include "../bswinfra/source/AsyncQueue.h"
#include "../bswinfra/source/SafeList.h"

using namespace bswi::event;

namespace tkm::monitor
{

class Registry : public std::enable_shared_from_this<Registry>
{
public:
  enum class Action { CollectAndSend };
  typedef struct Request {
    Action action;
    std::shared_ptr<ICollector> collector;
  } Request;

public:
  explicit Registry(std::shared_ptr<Options> &options);
  ~Registry() = default;

public:
  Registry(Registry const &) = delete;
  void operator=(Registry const &) = delete;

public:
  auto getShared() -> std::shared_ptr<Registry> { return shared_from_this(); }
  void initFromProc(void);

  void addEntry(int pid);
  void remEntry(int pid);
  void remEntry(std::string &name);
  auto getEntry(int pid) -> const std::shared_ptr<ProcEntry>;
  auto getEntry(std::string &name) -> const std::shared_ptr<ProcEntry>;
  auto getRegistryList(void) -> bswi::util::SafeList<std::shared_ptr<ProcEntry>> &
  {
    return m_list;
  }

  auto pushRequest(Registry::Request &request) -> int;
  void enableEvents();

private:
  bool requestHandler(const Request &request);
  auto getProcNameForPID(int pid) -> std::string;
  bool isBlacklisted(const std::string &name);

private:
  bswi::util::SafeList<std::shared_ptr<ProcEntry>> m_list{"RegistryList"};
  std::shared_ptr<AsyncQueue<Request>> m_queue = nullptr;
  std::shared_ptr<Options> m_options = nullptr;
  unsigned int m_usecPollInterval = 0;
};

} // namespace tkm::monitor
