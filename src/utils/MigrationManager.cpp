#include "utils/MigrationManager.h"
#include "utils/Logger.h"
#include <regex>
#include <fstream>
#include <sstream>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

namespace utils {

    MigrationManager& MigrationManager::getInstance() {
        static MigrationManager instance;
        return instance;
    }

    void MigrationManager::migrate() {
        TRADING_LOG_INFO("Starting database migration");
        
        ensureSchemaVersionTableExists();
        auto currentVersion = getCurrentVersion();
        auto migrations = loadMigrationFiles();

        for (const auto& migration : migrations) {
            if (migration.version <= currentVersion) {
                TRADING_LOG_DEBUG("Skipping migration {}: already applied", migration.version);
                continue;
            }

            TRADING_LOG_INFO("Applying migration {}: {}", migration.version, migration.description);
            auto start = std::chrono::steady_clock::now();
            
            try {
                executeSqlFile(migration);
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                
                recordMigration(migration, duration);
                TRADING_LOG_INFO("Migration {} completed in {} ms", migration.version, duration);
            } catch (const std::exception& e) {
                TRADING_LOG_ERROR("Migration {} failed: {}", migration.version, e.what());
                throw;
            }
        }
    }

    int MigrationManager::getCurrentVersion() {
        auto dbClient = drogon::app().getDbClient();
        try {
            auto result = dbClient->execSqlSync(
                "SELECT MAX(version) as version FROM schema_version WHERE success = true"
            );
            
            if (result.empty() || result[0]["version"].isNull()) {
                return 0;
            }
            
            return result[0]["version"].as<int>();
        } catch (const std::exception& e) {
            TRADING_LOG_ERROR("Failed to get current schema version: {}", e.what());
            return 0;
        }
    }

    std::vector<MigrationManager::MigrationFile> MigrationManager::loadMigrationFiles() {
        std::vector<MigrationFile> files;
        std::regex filePattern(R"(V(\d+)__(.+)\.sql)");

        for (const auto& entry : fs::directory_iterator("migrations")) {
            if (entry.path().extension() != ".sql") continue;

            std::smatch matches;
            std::string filename = entry.path().filename().string();
            
            if (std::regex_match(filename, matches, filePattern)) {
                MigrationFile file;
                file.version = std::stoi(matches[1].str());
                file.description = matches[2].str();
                file.path = entry.path().string();

                // 파일 내용 읽기 및 체크섬 계산
                std::ifstream t(file.path);
                std::stringstream buffer;
                buffer << t.rdbuf();
                file.checksum = calculateChecksum(buffer.str());

                files.push_back(file);
            }
        }

        std::sort(files.begin(), files.end(), 
            [](const MigrationFile& a, const MigrationFile& b) {
                return a.version < b.version;
            });

        return files;
    }

    int MigrationManager::calculateChecksum(const std::string& content) {
        int checksum = 0;
        for (char c : content) {
            checksum = ((checksum << 5) + checksum) + c;
        }
        return checksum;
    }

    void MigrationManager::ensureSchemaVersionTableExists() {
        auto dbClient = drogon::app().getDbClient();
        const std::string sql = R"(
            CREATE TABLE IF NOT EXISTS schema_version (
                version INTEGER PRIMARY KEY,
                description VARCHAR(200) NOT NULL,
                type VARCHAR(20) NOT NULL,
                script VARCHAR(1000) NOT NULL,
                checksum INTEGER NOT NULL,
                installed_by VARCHAR(100) NOT NULL,
                installed_on TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
                execution_time INTEGER NOT NULL,
                success BOOLEAN NOT NULL
            )
        )";

        try {
            dbClient->execSqlSync(sql);
        } catch (const std::exception& e) {
            TRADING_LOG_ERROR("Failed to create schema_version table: {}", e.what());
            throw;
        }
    }

    void MigrationManager::executeSqlFile(const MigrationFile& file) {
        auto dbClient = drogon::app().getDbClient();
        
        std::ifstream t(file.path);
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string sql = buffer.str();

        try {
            dbClient->execSqlSync(sql);
        } catch (const std::exception& e) {
            TRADING_LOG_ERROR("Failed to execute migration file {}: {}", file.path, e.what());
            throw;
        }
    }

    void MigrationManager::recordMigration(const MigrationFile& file, int executionTime) {
        auto dbClient = drogon::app().getDbClient();
        
        const std::string sql = R"(
            INSERT INTO schema_version 
            (version, description, type, script, checksum, installed_by, execution_time, success)
            VALUES
            ($1, $2, $3, $4, $5, $6, $7, $8)
        )";

        try {
            dbClient->execSqlSync(sql, 
                file.version,
                file.description,
                "SQL",
                file.path,
                file.checksum,
                "system",
                executionTime,
                true
            );
        } catch (const std::exception& e) {
            TRADING_LOG_ERROR("Failed to record migration {}: {}", file.version, e.what());
            throw;
        }
    }

} // namespace utils