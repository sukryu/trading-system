#include "secure/CryptoAuditor.h"
#include "utils/Logger.h"
#include "utils/JsonUtils.h"
#include <sstream>

void CryptoAuditor::logCryptoOperation(
    const std::string& operation,
    const std::string& keyId,
    bool success,
    const std::string& userId,
    const std::string& sourceIp
) {
    try {
        auto dbClient = drogon::app().getDbClient();
        auto now = std::chrono::system_clock::now();

        // 작업 메타데이터 준비
        Json::Value metadata;
        metadata["timestamp"] = static_cast<Json::Int64>(
            std::chrono::system_clock::to_time_t(now)
        );
        metadata["operation"] = operation;
        metadata["success"] = success;
        metadata["source_ip"] = sourceIp;
        metadata["user_id"] = userId;

        // 데이터베이스에 로그 저장
        dbClient->execSqlSync(
            "INSERT INTO crypto_audit_logs "
            "(key_id, operation, success, user_id, source_ip, metadata, created_at) "
            "VALUES ($1, $2, $3, $4, $5, $6, CURRENT_TIMESTAMP)",
            keyId,
            operation,
            success,
            userId,
            sourceIp,
            utils::JsonUtils::toJsonString(metadata)
        );

        TRADING_LOG_INFO(
            "Crypto operation logged - Operation: {}, Key: {}, Success: {}, User: {}", 
            operation, keyId, success, userId
        );

        // 실패한 작업 모니터링
        if (!success) {
            detectSuspiciousActivity(operation, keyId, userId, sourceIp);
        }

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to log crypto operation: {}", e.what());
        // 로깅 실패는 예외를 발생시키지 않음
    }
}

void CryptoAuditor::alertOnFailure(
    const std::string& operation,
    const std::string& error,
    const std::string& keyId,
    const std::string& userId
) {
    try {
        // 알림 데이터 준비
        Json::Value alertData;
        alertData["operation"] = operation;
        alertData["error"] = error;
        alertData["key_id"] = keyId;
        alertData["user_id"] = userId;
        alertData["timestamp"] = static_cast<Json::Int64>(
            std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now()
            )
        );

        // 보안팀 알림
        notifySecurityTeam(alertData);

        // 데이터베이스에 알림 기록
        auto dbClient = drogon::app().getDbClient();
        dbClient->execSqlSync(
            "INSERT INTO crypto_alerts "
            "(operation, error_message, key_id, user_id, metadata, created_at) "
            "VALUES ($1, $2, $3, $4, $5, CURRENT_TIMESTAMP)",
            operation,
            error,
            keyId,
            userId,
            utils::JsonUtils::toJsonString(alertData)
        );

        TRADING_LOG_WARN(
            "Crypto operation failed - Operation: {}, Error: {}, Key: {}, User: {}", 
            operation, error, keyId, userId
        );

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to process crypto operation failure: {}", e.what());
    }
}

void CryptoAuditor::detectSuspiciousActivity(
    const std::string& operation,
    const std::string& keyId,
    const std::string& userId,
    const std::string& sourceIp
) {
    try {
        // 비정상적인 작업 빈도 확인
        if (isRateLimitExceeded(userId, operation)) {
            Json::Value alertData;
            alertData["type"] = "rate_limit_exceeded";
            alertData["user_id"] = userId;
            alertData["operation"] = operation;
            notifySecurityTeam(alertData);
        }

        // 알 수 없는 IP 주소 확인
        if (!isKnownIpAddress(userId, sourceIp)) {
            Json::Value alertData;
            alertData["type"] = "unknown_ip_address";
            alertData["user_id"] = userId;
            alertData["source_ip"] = sourceIp;
            notifySecurityTeam(alertData);
        }

        // 최근 실패 횟수 확인
        auto dbClient = drogon::app().getDbClient();
        auto result = dbClient->execSqlSync(
            "SELECT COUNT(*) as failure_count FROM crypto_audit_logs "
            "WHERE user_id = $1 AND success = false "
            "AND created_at > CURRENT_TIMESTAMP - INTERVAL '1 hour'",
            userId
        );

        if (!result.empty() && 
            result[0]["failure_count"].as<int>() >= SUSPICIOUS_FAILURE_THRESHOLD) {
            
            Json::Value alertData;
            alertData["type"] = "multiple_failures";
            alertData["user_id"] = userId;
            alertData["failure_count"] = result[0]["failure_count"].as<int>();
            notifySecurityTeam(alertData);
        }

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to detect suspicious activity: {}", e.what());
    }
}

bool CryptoAuditor::isRateLimitExceeded(
    const std::string& userId, 
    const std::string& operation
) {
    try {
        auto dbClient = drogon::app().getDbClient();
        auto result = dbClient->execSqlSync(
            "SELECT COUNT(*) as op_count FROM crypto_audit_logs "
            "WHERE user_id = $1 AND operation = $2 "
            "AND created_at > CURRENT_TIMESTAMP - INTERVAL '1 minute'",
            userId,
            operation
        );

        return !result.empty() && 
               result[0]["op_count"].as<int>() > MAX_OPERATIONS_PER_MINUTE;

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to check rate limit: {}", e.what());
        return false;  // 에러 시 기본적으로 false 반환
    }
}

bool CryptoAuditor::isKnownIpAddress(
    const std::string& userId, 
    const std::string& sourceIp
) {
    try {
        auto dbClient = drogon::app().getDbClient();
        auto result = dbClient->execSqlSync(
            "SELECT COUNT(*) as known_count FROM ip_whitelist "
            "WHERE user_id = $1 AND ip_address = $2 AND is_active = true",
            userId,
            sourceIp
        );

        return !result.empty() && 
               result[0]["known_count"].as<int>() > 0;

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to check known IP address: {}", e.what());
        return false;  // 에러 시 기본적으로 신뢰하지 않는 것으로 간주
    }
}

void CryptoAuditor::notifySecurityTeam(const Json::Value& alertData) {
    // TODO: 실제 알림 시스템 구현
    // 1. 이메일 알림
    // 2. SMS 알림
    // 3. 모니터링 시스템 연동
    TRADING_LOG_WARN("Security Alert: {}", utils::JsonUtils::toJsonString(alertData));
}