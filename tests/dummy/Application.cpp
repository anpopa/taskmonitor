/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Application Class
 * @details   Main application class
 *-
 */

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "../tests/dummy/Application.h"

namespace tkm::monitor
{

Application *Application::appInstance = nullptr;

Application::Application(const std::string &name,
                         const std::string &description,
                         const std::string &configFile)
: bswi::app::IApplication(name, description)
{
  if (Application::appInstance != nullptr) {
    throw bswi::except::SingleInstance();
  }
  appInstance = this;
  m_options = std::make_shared<Options>(configFile);

  m_actionQueue = std::make_shared<AsyncQueue<Application::Action>>(
      "AppActionQueue", [this](const Action &action) { return actionHandler(action); });
  addEventSource(m_actionQueue);
}

auto Application::actionHandler(const Action &action) -> bool
{
  switch (action) {
  case Application::Action::ResetProcEvent:
#ifdef WITH_PROC_EVENT
    if (m_options->getFor(Options::Key::EnableProcEvent) ==
        tkmDefaults.valFor(Defaults::Val::True)) {
      m_procEvent = std::make_shared<ProcEvent>(m_options);
      m_procEvent->setEventSource();
    } else {
      logWarn() << "ProcEvent not enabled in configuration";
    }
#else
    logError() << "ProcEvent build time disabled";
#endif
    break;
  case Application::Action::SyncProcList:
    if (m_procRegistry != nullptr) {
      m_procRegistry->updateProcessList();
    }
    break;
  default:
    break;
  }

  logError() << "Unknown action request";
  return false;
}

} // namespace tkm::monitor
