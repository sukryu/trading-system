#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <json/json.h>

class SecurityPolicy {
public:
    static SecurityPolicy& getInstance() {
        static SecurityPolicy instance;
        return instance;
    }

    // 암호화 정책
    bool validateEncryptionAlgorithm(const std::string& algorithm);
    bool validateKeyLength(size_t length);
    bool validateKeyRotationInterval(const std::string& keyId);

    // 액세스 정책
    bool validateAccessControl(const std::string& userId, const std::string& operation);
    bool validateIpWhitelist(const std::string& userId, const std::string& sourceIp);
    bool enforceRateLimit(const std::string& userId, const std::string& operation);

    // 데이터 무결성 정책
    bool validateDataEncryptionState(const std::string& dataId);

    // 보안 감사 정책
    void logSensitiveOperation(const std::string& operation, const std::string& userId, const std::string& details);
    void detectSuspiciousActivity(const std::string& userId, const std::string& sourceIp);

    // 설정 메서드
    void setEncryptionPolicy(const Json::Value& policyConfig);
    void setAccessControlPolicy(const Json::Value& policyConfig);

private:
    SecurityPolicy() = default;
    ~SecurityPolicy() = default;
    SecurityPolicy(const SecurityPolicy&) = delete;
    SecurityPolicy& operator=(const SecurityPolicy&) = delete;

    // 내부 정책 설정
    Json::Value encryptionPolicy;
    Json::Value accessControlPolicy;

    // IP 화이트리스트 캐시
    std::unordered_map<std::string, std::vector<std::string>> ipWhitelistCache;

    // 비율 제한 관리
    std::unordered_map<std::string, std::chrono::system_clock::time_point> rateLimitTracker;

    static constexpr size_t MIN_KEY_LENGTH = 256; // 최소 키 길이 (비트 단위)
    static constexpr size_t RATE_LIMIT_THRESHOLD = 60; // 초당 요청 제한
};
