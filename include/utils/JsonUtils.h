#pragma once

#include <json/json.h>
#include <string>

namespace utils {

    class JsonUtils {
    public:
        static std::string toJsonString(const Json::Value& value) {
            Json::FastWriter writer;
            return writer.write(value);
        }

        static Json::Value parseJson(const std::string& jsonStr) {
            Json::Value root;
            Json::Reader reader;
            if (!reader.parse(jsonStr, root)) {
                throw std::runtime_error("Failed to parse JSON string");
            }
            return root;
        }
    };

} // namespace utils