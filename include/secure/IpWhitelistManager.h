#ifndef IP_WHITELIST_MANAGER_H
#define IP_WHITELIST_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include <drogon/orm/DbClient.h>

class IpWhitelistManager {
public:
    explicit IpWhitelistManager(const drogon::orm::DbClientPtr& dbClient);

    bool addIp(const std::string& userId, const std::string& ipAddress, const std::string& description = "");
    bool removeIp(const std::string& userId, const std::string& ipAddress);
    std::optional<std::string> validateIp(const std::string& userId, const std::string& ipAddress);
    std::vector<std::string> getUserIps(const std::string& userId);

private:
    drogon::orm::DbClientPtr dbClient_;
};

#endif // IP_WHITELIST_MANAGER_H
