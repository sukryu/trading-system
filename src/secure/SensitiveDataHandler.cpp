#include "secure/SensitiveDataHandler.h"

std::unordered_set<std::string> SensitiveDataHandler::sensitiveFields_ = {
    "apiKey", "secret", "password", "privateKey", "accessToken", "refreshToken"
};
std::mutex SensitiveDataHandler::fieldsMutex_;

void SensitiveDataHandler::addSensitiveField(const std::string& fieldName) {
    std::lock_guard<std::mutex> lock(fieldsMutex_);
    if (sensitiveFields_.find(fieldName) != sensitiveFields_.end()) {
        throw std::invalid_argument("Field '" + fieldName + "' is already marked as sensitive");
    }
    sensitiveFields_.insert(fieldName);
}

void SensitiveDataHandler::removeSensitiveField(const std::string& fieldName) {
    std::lock_guard<std::mutex> lock(fieldsMutex_);
    sensitiveFields_.erase(fieldName);
}

bool SensitiveDataHandler::isSensitiveField(const std::string& fieldName) {
    std::lock_guard<std::mutex> lock(fieldsMutex_);
    return sensitiveFields_.find(fieldName) != sensitiveFields_.end();
}

bool SensitiveDataHandler::containsSensitiveData(const Json::Value& data) {
    if (data.isObject()) {
        std::lock_guard<std::mutex> lock(fieldsMutex_);
        for (const auto& field : sensitiveFields_) {
            if (data.isMember(field)) return true;
        }

        for (const auto& key : data.getMemberNames()) {
            if (data[key].isObject() && containsSensitiveData(data[key])) {
                return true;
            }
        }
    }
    return false;
}

Json::Value SensitiveDataHandler::sanitizeForLog(const Json::Value& data) {
    if (!data.isObject()) return data;

    Json::Value sanitized = data;
    for (const auto& key : data.getMemberNames()) {
        if (data[key].isObject()) {
            sanitized[key] = sanitizeForLog(data[key]);
        } else if (isSensitiveField(key)) {
            sanitized[key] = maskString(data[key].asString());
        }
    }
    return sanitized;
}

std::vector<std::string> SensitiveDataHandler::findExposedSensitiveFields(const Json::Value& data) {
    std::vector<std::string> exposedFields;
    if (data.isObject()) {
        std::lock_guard<std::mutex> lock(fieldsMutex_);
        for (const auto& field : sensitiveFields_) {
            if (data.isMember(field)) {
                exposedFields.push_back(field);
            }
        }
        for (const auto& key : data.getMemberNames()) {
            if (data[key].isObject()) {
                auto nestedExposed = findExposedSensitiveFields(data[key]);
                exposedFields.insert(exposedFields.end(), nestedExposed.begin(), nestedExposed.end());
            }
        }
    }
    return exposedFields;
}

void SensitiveDataHandler::validateSensitiveData(const Json::Value& data, int depth) {
    if (depth > MAX_DEPTH) return;
    if (!data.isObject()) return;

    for (const auto& key : data.getMemberNames()) {
        if (data[key].isObject()) {
            validateSensitiveData(data[key], depth + 1);
        } else if (isSensitiveField(key)) {
            validateFieldValue(key, data[key].asString());
        }
    }
}

void SensitiveDataHandler::validateFieldValue(const std::string& field, const std::string& value) {
    if (field == "apiKey") {
        validateApiKeyFormat(value);
    } else if (field == "secret") {
        validateSecretFormat(value);
    }
}

void SensitiveDataHandler::validateApiKeyFormat(const std::string& key) {
    static const std::regex apiKeyPattern("^[A-Za-z0-9]{32,64}$");
    if (!std::regex_match(key, apiKeyPattern)) {
        throw std::invalid_argument("Invalid API key format: " + key);
    }
}

void SensitiveDataHandler::validateSecretFormat(const std::string& secret) {
    if (secret.length() < 32) {
        throw std::invalid_argument("Secret must be at least 32 characters long");
    }
    static const std::regex secretPattern(
        "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{32,}$"
    );
    if (!std::regex_match(secret, secretPattern)) {
        throw std::invalid_argument(
            "Secret must contain uppercase, lowercase, number and special characters"
        );
    }
}

std::string SensitiveDataHandler::maskString(const std::string& str) {
    if (str.empty()) return "";
    if (str.size() <= 8) {
        return std::string(str.size(), '*');
    }
    return str.substr(0, 4) + std::string(str.size() - 8, '*') + str.substr(str.size() - 4);
}
