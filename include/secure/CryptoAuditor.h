#pragma once

#include <string>
#include <chrono>
#include <json/json.h>
#include <drogon/drogon.h>

class CryptoAuditor {
public:
    static CryptoAuditor& getInstance() {
        static CryptoAuditor instance;
        return instance;
    }

    // 암호화 작업 로깅
    void logCryptoOperation(
        const std::string& operation,
        const std::string& keyId,
        bool success,
        const std::string& userId = "",
        const std::string& sourceIp = ""
    );

    // 실패 알림
    void alertOnFailure(
        const std::string& operation,
        const std::string& error,
        const std::string& keyId = "",
        const std::string& userId = ""
    );

    // 의심스러운 활동 감지 및 보고
    void detectSuspiciousActivity(
        const std::string& operation,
        const std::string& keyId,
        const std::string& userId,
        const std::string& sourceIp
    );

    // 감사 로그 조회
    std::vector<Json::Value> getAuditLogs(
        const std::string& keyId,
        const std::chrono::system_clock::time_point& startTime,
        const std::chrono::system_clock::time_point& endTime
    );

private:
    CryptoAuditor() = default;
    ~CryptoAuditor() = default;
    CryptoAuditor(const CryptoAuditor&) = delete;
    CryptoAuditor& operator=(const CryptoAuditor&) = delete;

    // 내부 헬퍼 메서드
    bool isRateLimitExceeded(const std::string& userId, const std::string& operation);
    void recordOperationAttempt(const std::string& userId, const std::string& operation);
    bool isKnownIpAddress(const std::string& userId, const std::string& sourceIp);
    void notifySecurityTeam(const Json::Value& alertData);
    
    static constexpr size_t MAX_OPERATIONS_PER_MINUTE = 60;
    static constexpr size_t SUSPICIOUS_FAILURE_THRESHOLD = 5;
};