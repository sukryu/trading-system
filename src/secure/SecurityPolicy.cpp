#include "secure/SecurityPolicy.h"
#include "utils/Logger.h"
#include <stdexcept>
#include <chrono>

bool SecurityPolicy::validateEncryptionAlgorithm(const std::string& algorithm) {
    static const std::vector<std::string> supportedAlgorithms = {
        "AES-256-GCM", "RSA-OAEP", "ChaCha20-Poly1305"
    };

    if (std::find(supportedAlgorithms.begin(), supportedAlgorithms.end(), algorithm) != supportedAlgorithms.end()) {
        return true;
    }

    TRADING_LOG_WARN("Unsupported encryption algorithm: {}", algorithm);
    return false;
}

bool SecurityPolicy::validateKeyLength(size_t length) {
    if (length >= MIN_KEY_LENGTH) {
        return true;
    }

    TRADING_LOG_WARN("Key length too short: {} bits (minimum: {} bits)", length, MIN_KEY_LENGTH);
    return false;
}

bool SecurityPolicy::validateKeyRotationInterval(const std::string& keyId) {
    auto lastRotation = encryptionPolicy["keyRotation"][keyId].asUInt64();
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::hours>(
        now.time_since_epoch()).count() - lastRotation;

    if (duration > encryptionPolicy["rotationIntervalHours"].asUInt64()) {
        TRADING_LOG_WARN("Key rotation overdue for key: {}", keyId);
        return false;
    }
    return true;
}

bool SecurityPolicy::validateAccessControl(const std::string& userId, const std::string& operation) {
    auto roles = accessControlPolicy["roles"];
    if (!roles.isMember(userId)) {
        TRADING_LOG_WARN("Access denied: User {} not found in roles", userId);
        return false;
    }

    auto allowedOperations = roles[userId]["allowed_operations"];
    if (std::find(allowedOperations.begin(), allowedOperations.end(), operation) == allowedOperations.end()) {
        TRADING_LOG_WARN("Access denied: User {} not authorized for operation {}", userId, operation);
        return false;
    }

    return true;
}

bool SecurityPolicy::validateIpWhitelist(const std::string& userId, const std::string& sourceIp) {
    if (ipWhitelistCache.find(userId) == ipWhitelistCache.end()) {
        TRADING_LOG_WARN("IP whitelist not found for user: {}", userId);
        return false;
    }

    const auto& whitelist = ipWhitelistCache[userId];
    if (std::find(whitelist.begin(), whitelist.end(), sourceIp) == whitelist.end()) {
        TRADING_LOG_WARN("Access denied: IP {} not whitelisted for user {}", sourceIp, userId);
        return false;
    }

    return true;
}

bool SecurityPolicy::enforceRateLimit(const std::string& userId, const std::string& operation) {
    auto now = std::chrono::system_clock::now();
    auto& lastRequestTime = rateLimitTracker[userId + operation];

    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastRequestTime).count() < RATE_LIMIT_THRESHOLD) {
        TRADING_LOG_WARN("Rate limit exceeded for user: {}, operation: {}", userId, operation);
        return false;
    }

    lastRequestTime = now;
    return true;
}

void SecurityPolicy::logSensitiveOperation(const std::string& operation, const std::string& userId, const std::string& details) {
    TRADING_LOG_INFO("Sensitive operation logged - Operation: {}, User: {}, Details: {}", operation, userId, details);
}

void SecurityPolicy::detectSuspiciousActivity(const std::string& userId, const std::string& sourceIp) {
    TRADING_LOG_WARN("Suspicious activity detected - User: {}, IP: {}", userId, sourceIp);
}

void SecurityPolicy::setEncryptionPolicy(const Json::Value& policyConfig) {
    encryptionPolicy = policyConfig;
}

void SecurityPolicy::setAccessControlPolicy(const Json::Value& policyConfig) {
    accessControlPolicy = policyConfig;

    for (const auto& user : policyConfig["ipWhitelist"].getMemberNames()) {
        std::vector<std::string> whitelist;
        for (const auto& ip : policyConfig["ipWhitelist"][user]) {
            if (ip.isString()) {
                whitelist.push_back(ip.asString());
            }
        }
        ipWhitelistCache[user] = whitelist;
    }
}
