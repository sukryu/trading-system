#include "secure/IpWhitelistManager.h"
#include "utils/Logger.h"
#include <drogon/drogon.h>

IpWhitelistManager::IpWhitelistManager(const drogon::orm::DbClientPtr& dbClient)
    : dbClient_(dbClient) {}

bool IpWhitelistManager::addIp(const std::string& userId, const std::string& ipAddress, const std::string& description) {
    try {
        dbClient_->execSqlSync(
            "INSERT INTO ip_whitelist (user_id, ip_address, description) "
            "VALUES ($1, $2, $3) ON CONFLICT (user_id, ip_address) DO UPDATE "
            "SET is_active = true, updated_at = CURRENT_TIMESTAMP",
            userId, ipAddress, description
        );
        return true;
    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to add IP to whitelist: {}", e.what());
        return false;
    }
}

bool IpWhitelistManager::removeIp(const std::string& userId, const std::string& ipAddress) {
    try {
        dbClient_->execSqlSync(
            "UPDATE ip_whitelist SET is_active = false, updated_at = CURRENT_TIMESTAMP "
            "WHERE user_id = $1 AND ip_address = $2",
            userId, ipAddress
        );
        return true;
    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to remove IP from whitelist: {}", e.what());
        return false;
    }
}

std::optional<std::string> IpWhitelistManager::validateIp(const std::string& userId, const std::string& ipAddress) {
    try {
        auto result = dbClient_->execSqlSync(
            "SELECT ip_address FROM ip_whitelist WHERE user_id = $1 AND ip_address = $2 AND is_active = true",
            userId, ipAddress
        );

        if (!result.empty()) {
            return result[0]["ip_address"].as<std::string>();
        }
        return std::nullopt;

    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to validate IP: {}", e.what());
        return std::nullopt;
    }
}

std::vector<std::string> IpWhitelistManager::getUserIps(const std::string& userId) {
    std::vector<std::string> ips;
    try {
        auto result = dbClient_->execSqlSync(
            "SELECT ip_address FROM ip_whitelist WHERE user_id = $1 AND is_active = true",
            userId
        );

        for (const auto& row : result) {
            ips.push_back(row["ip_address"].as<std::string>());
        }
    } catch (const std::exception& e) {
        TRADING_LOG_ERROR("Failed to fetch user IPs: {}", e.what());
    }

    return ips;
}
