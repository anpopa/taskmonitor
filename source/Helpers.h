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

#pragma once

#include <cstdint>
#include <string>

namespace tkm
{

auto getContextName(const std::string &contPath, uint64_t ctxId) -> std::string;

} // namespace tkm
