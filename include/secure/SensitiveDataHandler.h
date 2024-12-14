#pragma once
#include <json/json.h>
#include <string>
#include <unordered_set>
#include <mutex>
#include <vector>
#include <regex>
#include <stdexcept>

class SensitiveDataHandler {
public:

    // 민감 필드 관리
    static void addSensitiveField(const std::string& fieldName);
    static void removeSensitiveField(const std::string& fieldName);
    static bool isSensitiveField(const std::string& fieldName);
    
    // 기존 메서드
    static void maskSensitiveData(Json::Value& data);
    
    // 민감 데이터 처리
    static bool containsSensitiveData(const Json::Value& data);
    static Json::Value sanitizeForLog(const Json::Value& data);
    static std::vector<std::string> findExposedSensitiveFields(const Json::Value& data);

    // 민감 데이터 검증증
    static void validateSensitiveData(const Json::Value& data, int depth = 0);

private:
    static std::unordered_set<std::string> sensitiveFields_;
    static std::mutex fieldsMutex_;  // thread-safe를 위한 뮤텍스

    // 내부 유틸리티 메서드들
    static std::string maskString(const std::string& str);
    static void validateFieldValue(const std::string& field, const std::string& value);
    static void validateApiKeyFormat(const std::string& key);
    static void validateSecretFormat(const std::string& secret);

    // 초기 민감 필드 설정
    static void initializeSensitiveFields();

    static constexpr int MAX_DEPTH = 10; // JSON 처리 최대 깊이
};