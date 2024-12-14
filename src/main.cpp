#include <drogon/drogon.h>
#include <iostream>
#include <filesystem>
#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/MigrationManager.h"

namespace fs = std::filesystem;

int main() {
    try {
        // 환경 변수 로드
        auto& config = utils::Config::getInstance();
        config.loadEnvironment();
        
        // Drogon 설정 파일 로드
        config.loadDrogonConfig("drogon.json");

        // 로그 디렉토리 경로 설정 및 로거 초기화
        fs::path logPath = fs::path(config.getProjectRoot()) / "logs";
        utils::Logger::init(logPath.string());
        utils::Logger::setLogLevel(config.getLogLevel());

        TRADING_LOG_INFO("Starting trading system server...");
        TRADING_LOG_INFO("Environment: {}", config.getEnvironment());
        TRADING_LOG_INFO("Log Level: {}", config.getLogLevel());

        // Drogon 앱 설정
        auto& app = drogon::app();
        
        // JSON 설정을 Drogon에 적용
        app.setLogPath(logPath.string())
           .setLogLevel(trantor::Logger::LogLevel::kInfo)
           .addListener("0.0.0.0", 8000)
           .setThreadNum(16);

        // Drogon 설정 파일 로드 (DB 설정 포함)
        app.loadConfigFile(config.findConfigFile("drogon.json"));

        // DB 마이그레이션 실행 (이제 DB 설정이 로드된 후)
        auto& mgt = utils::MigrationManager::getInstance();
        mgt.migrate();
        
        // 서버 시작 메시지
        std::cout << "\n==================================" << std::endl;
        std::cout << "Server starting on http://0.0.0.0:8000" << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        std::cout << "==================================\n" << std::endl;
        
        TRADING_LOG_INFO("Server starting on http://0.0.0.0:8000");
        
        // 서버 시작
        app.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (utils::Logger::getLogger()) {
            TRADING_LOG_ERROR("Server startup failed: {}", e.what());
        }
        return 1;
    }
}