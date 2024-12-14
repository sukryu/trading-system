#pragma once

#include <string>
#include <vector>
#include <drogon/drogon.h>
#include <filesystem>

namespace utils {

    class MigrationManager {
    public:
        static MigrationManager& getInstance();

        // 마이그레이션 실행
        void migrate();

        // 현재 스키마 버전 조회
        int getCurrentVersion();

    private:
        MigrationManager() = default;
        ~MigrationManager() = default;
        MigrationManager(const MigrationManager&) = delete;
        MigrationManager& operator=(const MigrationManager&) = delete;

        // 마이그레이션 파일 정보
        struct MigrationFile {
            int version;
            std::string description;
            std::string path;
            int checksum;
        };

        // 마이그레이션 파일 로드
        std::vector<MigrationFile> loadMigrationFiles();

        // 체크섬 계산
        int calculateChecksum(const std::string& content);

        // 마이그레이션 테이블 생성
        void ensureSchemaVersionTableExists();

        // SQL 파일 실행
        void executeSqlFile(const MigrationFile& file);

        // 마이그레이션 이력 기록
        void recordMigration(const MigrationFile& file, int executionTime);
    };

}