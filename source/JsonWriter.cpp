/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     JsonWriter Class
 * @details   Write Json Values to different streams
 *-
 */

#include "JsonWriter.h"
#include "../bswinfra/source/Logger.h"
#include <json/writer.h>
#include <memory>

namespace tkm::monitor
{

JsonWriter *JsonWriter::instance = nullptr;

JsonWriter::JsonWriter()
{
    builder["commentStyle"] = "None";
    builder["indentation"] = "";
}

void JsonWriter::Payload::print()
{
    logInfo() << m_stream.str();
}

} // namespace tkm::monitor
