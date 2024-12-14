#include "utils/Logger.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
namespace utils {

    std::shared_ptr<spdlog::logger> Logger::logger_;

    void Logger::init(const std::string& logPath) {
        try {
            // 로그 디렉토리가 없으면 생성
            fs::path logDir = fs::path(logPath);
            if (!fs::exists(logDir)) {
                fs::create_directories(logDir);
            }

            // 로그 파일 경로 설정
            std::string logFile = (logDir / "trading_system.log").string();

            // 로테이팅 파일 로거 생성
            logger_ = spdlog::rotating_logger_mt("trading_system",
                                            logFile,
                                            MAX_FILE_SIZE,
                                            MAX_FILES);

            // 기본 로그 레벨 설정
            logger_->set_level(spdlog::level::debug);

            // 로그 패턴 설정
            logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");

            // 매 초마다 로그를 플러시
            spdlog::flush_every(std::chrono::seconds(1));

            TRADING_LOG_INFO("Logger initialized successfully");
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
            throw;
        }
    }

    std::shared_ptr<spdlog::logger> Logger::getLogger() {
        if (!logger_) {
            throw std::runtime_error("Logger not initialized. Call Logger::init() first.");
        }
        return logger_;
    }

    void Logger::setLogLevel(const std::string& level) {
        if (!logger_) {
            throw std::runtime_error("Logger not initialized. Call Logger::init() first.");
        }

        if (level == "TRACE") {
            logger_->set_level(spdlog::level::trace);
        } else if (level == "DEBUG") {
            logger_->set_level(spdlog::level::debug);
        } else if (level == "INFO") {
            logger_->set_level(spdlog::level::info);
        } else if (level == "WARN") {
            logger_->set_level(spdlog::level::warn);
        } else if (level == "ERROR") {
            logger_->set_level(spdlog::level::err);
        } else {
            throw std::invalid_argument("Invalid log level: " + level);
        }

        TRADING_LOG_INFO("Log level set to: {}", level);
    }

}