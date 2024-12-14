#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace utils {

    class Logger {
    public:
        static void init(const std::string& logPath);
        static std::shared_ptr<spdlog::logger> getLogger();
        static void setLogLevel(const std::string& level);

    private:
        static std::shared_ptr<spdlog::logger> logger_;
        static constexpr size_t MAX_FILE_SIZE = 5 * 1024 * 1024;  // 5MB
        static constexpr size_t MAX_FILES = 3;
    };

    // 매크로 정의
    #define TRADING_LOG_TRACE(...) SPDLOG_LOGGER_TRACE(utils::Logger::getLogger(), __VA_ARGS__)
    #define TRADING_LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(utils::Logger::getLogger(), __VA_ARGS__)
    #define TRADING_LOG_INFO(...)  SPDLOG_LOGGER_INFO(utils::Logger::getLogger(), __VA_ARGS__)
    #define TRADING_LOG_WARN(...)  SPDLOG_LOGGER_WARN(utils::Logger::getLogger(), __VA_ARGS__)
    #define TRADING_LOG_ERROR(...) SPDLOG_LOGGER_ERROR(utils::Logger::getLogger(), __VA_ARGS__)

}