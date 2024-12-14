#include "models/User.h"
#include <stdexcept>

namespace models {

    Json::Value User::toJson() const {
        Json::Value json;
        json["id"] = static_cast<Json::Int64>(id_);
        json["email"] = email_;
        json["username"] = username_;
        json["is_active"] = is_active_;
        json["last_login_at"] = last_login_at_.toFormattedString(false);
        json["created_at"] = created_at_.toFormattedString(false);
        json["updated_at"] = updated_at_.toFormattedString(false);
        return json;
    }

    void User::fromJson(const Json::Value& json) {
        if (json.isMember("id")) {
            id_ = json["id"].asInt64();
        }

        if (!json.isMember("email") || !json.isMember("username")) {
            throw std::invalid_argument("Missing required fields in User JSON");
        }

        email_ = json["email"].asString();
        username_ = json["username"].asString();
        
        if (json.isMember("password_hash")) {
            password_hash_ = json["password_hash"].asString();
        }

        if (json.isMember("is_active")) {
            is_active_ = json["is_active"].asBool();
        }
        
        if (json.isMember("last_login_at")) {
            last_login_at_ = trantor::Date::fromDbString(json["last_login_at"].asString());
        }
        
        if (json.isMember("created_at")) {
            created_at_ = trantor::Date::fromDbString(json["created_at"].asString());
        }
        
        if (json.isMember("updated_at")) {
            updated_at_ = trantor::Date::fromDbString(json["updated_at"].asString());
        }
    }

    User User::fromDbRow(const drogon::orm::Row& row) {
        User user;
        user.setId(row["id"].as<int64_t>());
        user.setEmail(row["email"].as<std::string>());
        user.setUsername(row["username"].as<std::string>());
        user.setPasswordHash(row["password_hash"].as<std::string>());
        user.setIsActive(row["is_active"].as<bool>());
        user.setLastLoginAt(trantor::Date::fromDbString(row["last_login_at"].as<std::string>()));
        user.setCreatedAt(trantor::Date::fromDbString(row["created_at"].as<std::string>()));
        user.setUpdatedAt(trantor::Date::fromDbString(row["updated_at"].as<std::string>()));
        return user;
    }

    std::vector<User> User::fromDbResult(const drogon::orm::Result& result) {
        std::vector<User> users;
        users.reserve(result.size());
        
        for (const auto& row : result) {
            users.push_back(fromDbRow(row));
        }
        
        return users;
    }

} // namespace models