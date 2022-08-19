/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Helper methods
 * @details   Verious helper methods
 *-
 */

#include "Helpers.h"
#include <TaskMonitor.h>
#ifdef WITH_LXC
#include <lxc/lxccontainer.h>
#endif

namespace tkm
{

#ifdef WITH_LXC
static auto getContainerNameForContext(const std::string &contPath, uint64_t ctxId) -> std::string
{
  struct lxc_container **activeContainers = NULL;
  std::string contName{"unknown"};
  char **names = NULL;
  bool found = false;

  auto count = list_active_containers(contPath.c_str(), &names, &activeContainers);
  for (int i = 0; i < count && !found; i++) {
    struct lxc_container *container = activeContainers[i];
    const char *name = names[i];

    if (name == NULL || container == NULL) {
      continue;
    }

    if (container->is_running(container)) {
      auto pid = container->init_pid(container);
      if (getContextId(pid) == ctxId) {
        contName = std::string(name);
        found = true;
      }
    }
  }

  for (int i = 0; i < count; i++) {
    free(names[i]);
    lxc_container_put(activeContainers[i]);
  }

  return contName;
}
#endif

auto getContextName(const std::string &contPath, uint64_t ctxId) -> std::string
{
  if (ctxId == tkm::getContextId(1)) {
    return std::string{"root"};
  }
#ifdef WITH_LXC
  return getContainerNameForContext(contPath, ctxId);
#else
  return std::string("unknown");
#endif
}

} // namespace tkm
