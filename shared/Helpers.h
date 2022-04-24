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

#include <any>
#include <string>

#include "../bswinfra/source/Exceptions.h"
#include "../bswinfra/source/Logger.h"

#include "Collector.pb.h"

namespace tkm
{

auto jnkHsh(const char *key) -> uint64_t;
auto base64Encode(unsigned char const *bytes_to_encode, unsigned int in_len) -> std::string;
auto base64Decode(std::string const &encoded_string) -> std::string;
bool sendCollectorDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor);
bool readCollectorDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor);

} // namespace tkm
