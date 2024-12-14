#pragma once

#include <string>
#include <memory>
#include <jsoncpp/json/json.h>
#include <filesystem>

namespace utils {

    class Config {
    public:
        static Config& getInstance() {
            static Config instance;
            return instance;
        }

        void loadEnvironment();
        void loadDrogonConfig(const std::string& configPath);
        std::string findConfigFile(const std::string& filename);
        void createRequiredDirectories();
        
        // Getters for configuration values
        int getServerPort() const { return serverPort; }
        std::string getLogLevel() const { return logLevel; }
        std::string getExchangeApiKey() const { return exchangeApiKey; }
        std::string getExchangeApiSecret() const { return exchangeApiSecret; }
        std::string getEnvironment() const { return environment; }
        Json::Value getDrogonConfig() const { return drogonConfig; }
        std::string getProjectRoot() const;

        // Database configuration getters
        std::string getDbHost() const { return dbHost; }
        int getDbPort() const { return dbPort; }
        std::string getDbName() const { return dbName; }
        std::string getDbUser() const { return dbUser; }
        std::string getDbPassword() const { return dbPassword; }
        
        // Security configuration getters
        std::string getSslCertPath() const { return sslCertPath; }
        std::string getSslKeyPath() const { return sslKeyPath; }
        bool isSslEnabled() const { return enableSsl; }
        std::string getEncryptionKey() const { return encryptionKey; }

    private:
        Config() = default;
        ~Config() = default;
        Config(const Config&) = delete;
        Config& operator=(const Config&) = delete;

        std::string getEnvVar(const std::string& key, const std::string& defaultValue = "");
        void validateEnvironmentVariables() const;
        void validateEncryptionKey() const;
        
        // Basic configuration values
        int serverPort{8080};
        std::string logLevel{"DEBUG"};
        std::string exchangeApiKey;
        std::string exchangeApiSecret;
        std::string environment{"development"};
        Json::Value drogonConfig;
        std::filesystem::path projectRoot;

        // Database configuration
        std::string dbHost{"127.0.0.1"};
        int dbPort{5432};
        std::string dbName{"trading_db"};
        std::string dbUser{"trading_user"};
        std::string dbPassword;

        // Security configuration
        bool enableSsl{false};
        std::string sslCertPath;
        std::string sslKeyPath;
        std::string encryptionKey;
    };
}