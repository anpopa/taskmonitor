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
}

} // namespace tkm::monitor
