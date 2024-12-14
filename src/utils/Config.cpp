#include "utils/Config.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

namespace utils {

    std::string Config::getEnvVar(const std::string& key, const std::string& defaultValue) {
        const char* val = std::getenv(key.c_str());
        return val ? std::string(val) : defaultValue;
    }

    void Config::loadEnvironment() {
        // Basic configuration
        serverPort = std::stoi(getEnvVar("SERVER_PORT", "8080"));
        logLevel = getEnvVar("LOG_LEVEL", "DEBUG");
        exchangeApiKey = getEnvVar("EXCHANGE_API_KEY", "");
        exchangeApiSecret = getEnvVar("EXCHANGE_API_SECRET", "");
        environment = getEnvVar("ENVIRONMENT", "development");

        // Database configuration
        dbHost = getEnvVar("DB_HOST", "127.0.0.1");
        dbPort = std::stoi(getEnvVar("DB_PORT", "5432"));
        dbName = getEnvVar("DB_NAME", "trading_db");
        dbUser = getEnvVar("DB_USER", "trading_user");
        dbPassword = getEnvVar("DB_PASSWORD", "");

        // Security configuration
        enableSsl = getEnvVar("ENABLE_SSL", "false") == "true";
        sslCertPath = getEnvVar("SSL_CERT_PATH", "");
        sslKeyPath = getEnvVar("SSL_KEY_PATH", "");
        encryptionKey = getEnvVar("ENCRYPTION_KEY", "");

        validateEnvironmentVariables();
        validateEncryptionKey();
    }

    void Config::validateEnvironmentVariables() const {
        std::vector<std::string> missingVars;

        // 필수 환경 변수 검증
        if (dbPassword.empty()) missingVars.push_back("DB_PASSWORD");
        if (encryptionKey.empty()) missingVars.push_back("ENCRYPTION_KEY");
        
        if (enableSsl) {
            if (sslCertPath.empty()) missingVars.push_back("SSL_CERT_PATH");
            if (sslKeyPath.empty()) missingVars.push_back("SSL_KEY_PATH");
        }

        if (!missingVars.empty()) {
            std::string errorMsg = "Missing required environment variables: ";
            for (const auto& var : missingVars) {
                errorMsg += var + " ";
            }
            throw std::runtime_error(errorMsg);
        }
    }

    void Config::validateEncryptionKey() const {
        if (encryptionKey.length() != 32) {
            throw std::runtime_error("ENCRYPTION_KEY must be exactly 32 bytes long");
        }
    }

    std::string Config::findConfigFile(const std::string& filename) {
        // 현재 실행 파일의 디렉토리에서 상위로 올라가며 설정 파일 찾기
        fs::path currentPath = fs::current_path();
        while (!currentPath.empty()) {
            fs::path configPath = currentPath / "config" / filename;
            if (fs::exists(configPath)) {
                // 설정 파일을 찾았을 때 프로젝트 루트 디렉토리 저장
                projectRoot = currentPath;
                return configPath.string();
            }
            
            // 상위 디렉토리로 이동
            fs::path parentPath = currentPath.parent_path();
            if (parentPath == currentPath) {
                break;
            }
            currentPath = parentPath;
        }
        
        throw std::runtime_error("Cannot find config file: " + filename);
    }

    void Config::createRequiredDirectories() {
        if (projectRoot.empty()) {
            throw std::runtime_error("Project root not set. Call findConfigFile first.");
        }

        // 필요한 디렉토리 생성
        std::vector<std::string> dirs = {"logs", "public", "uploads"};
        for (const auto& dir : dirs) {
            fs::path dirPath = projectRoot / dir;
            if (!fs::exists(dirPath)) {
                std::cout << "Creating directory: " << dirPath << std::endl;
                fs::create_directories(dirPath);
            }
        }
    }

    void Config::loadDrogonConfig(const std::string& /*configPath*/) {
        std::string fullConfigPath = findConfigFile("drogon.json");
        std::cout << "Loading config from: " << fullConfigPath << std::endl;
        
        createRequiredDirectories();
        
        std::ifstream config_file(fullConfigPath);
        if (!config_file.is_open()) {
            throw std::runtime_error("Cannot open config file: " + fullConfigPath);
        }

        Json::CharReaderBuilder builder;
        std::string errs;
        if (!Json::parseFromStream(builder, config_file, &drogonConfig, &errs)) {
            throw std::runtime_error("Failed to parse config file: " + errs);
        }

        // 환경 변수로 설정 덮어쓰기
        auto& dbClients = drogonConfig["db_clients"];
        if (!dbClients.empty()) {
            dbClients[0]["host"] = dbHost;
            dbClients[0]["port"] = dbPort;
            dbClients[0]["dbname"] = dbName;
            dbClients[0]["user"] = dbUser;
            dbClients[0]["passwd"] = dbPassword;
        }

        // SSL 설정
        if (enableSsl) {
            drogonConfig["ssl"] = Json::Value(Json::objectValue);
            drogonConfig["ssl"]["cert"] = sslCertPath;
            drogonConfig["ssl"]["key"] = sslKeyPath;
        }

        // 기본 설정 덮어쓰기
        auto& listeners = drogonConfig["listeners"];
        if (!listeners.empty() && !listeners[0].empty()) {
            listeners[0]["port"] = serverPort;
        }

        if (!drogonConfig["log"].empty()) {
            drogonConfig["log"]["log_level"] = logLevel;
        }
    }

    std::string Config::getProjectRoot() const {
        return projectRoot.string();
    }

}