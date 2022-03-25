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

#pragma once

#include <json/json.h>
#include <json/writer.h>
#include <sstream>
#include <string>

namespace tkm::monitor
{

class JsonWriter
{
public:
    static JsonWriter *getInstance() { return (!instance) ? instance = new JsonWriter : instance; }

    struct Payload {
        Payload() = default;
        ~Payload() { print(); }

        template <class T>
        Payload &operator<<(const T &val)
        {
            const std::unique_ptr<Json::StreamWriter> writer(
                JsonWriter::getInstance()->builder.newStreamWriter());
            writer->write(val, &m_stream);
            return *this;
        }

        void print();

    private:
        std::ostringstream m_stream;
        friend class JsonWriter;
    };

    static Payload write() { return Payload {}; }

public:
    Json::StreamWriterBuilder builder;

public:
    JsonWriter(JsonWriter const &) = delete;
    void operator=(JsonWriter const &) = delete;

private:
    JsonWriter();
    ~JsonWriter() = default;

private:
    static JsonWriter *instance;
};

} // namespace tkm::monitor

#define writeJsonStream() ::tkm::monitor::JsonWriter::write()
