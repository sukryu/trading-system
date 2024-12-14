#pragma once

#include "models/BaseModel.h"
#include <string>
#include <json/json.h>
#include <trantor/utils/Date.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Result.h>
#include <vector>

namespace models {

    class User : public BaseModel {
    public:
        User() = default;
        ~User() override = default;

        // Getters
        int64_t getId() const { return id_; }
        const std::string& getEmail() const { return email_; }
        const std::string& getUsername() const { return username_; }
        const std::string& getPasswordHash() const { return password_hash_; }
        bool isActive() const { return is_active_; }
        const trantor::Date& getLastLoginAt() const { return last_login_at_; }
        const trantor::Date& getCreatedAt() const { return created_at_; }
        const trantor::Date& getUpdatedAt() const { return updated_at_; }

        // Setters
        void setId(int64_t id) { id_ = id; }
        void setEmail(const std::string& email) { email_ = email; }
        void setUsername(const std::string& username) { username_ = username; }
        void setPasswordHash(const std::string& password_hash) { password_hash_ = password_hash; }
        void setIsActive(bool is_active) { is_active_ = is_active; }
        void setLastLoginAt(const trantor::Date& last_login_at) { last_login_at_ = last_login_at; }
        void setCreatedAt(const trantor::Date& created_at) { created_at_ = created_at; }
        void setUpdatedAt(const trantor::Date& updated_at) { updated_at_ = updated_at; }

        // JSON conversion
        Json::Value toJson() const override;
        void fromJson(const Json::Value& json) override;

        // DB conversion
        static User fromDbRow(const drogon::orm::Row& row);
        static std::vector<User> fromDbResult(const drogon::orm::Result& result);

    private:
        int64_t id_{0};
        std::string email_;
        std::string username_;
        std::string password_hash_;
        bool is_active_{true};
        trantor::Date last_login_at_;
        trantor::Date created_at_;
        trantor::Date updated_at_;
    };

} // namespace models