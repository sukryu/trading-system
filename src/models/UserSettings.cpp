#include "models/UserSettings.h"
#include "utils/JsonUtils.h"
#include <stdexcept>

namespace models {

    Json::Value UserSettings::toJson() const {
        Json::Value json;
        json["id"] = static_cast<Json::Int64>(id_);
        json["user_id"] = static_cast<Json::Int64>(user_id_);
        json["exchange_name"] = exchange_name_;
        json["api_credentials"] = api_credentials_;
        json["strategy_params"] = strategy_params_;
        json["watchlist"] = watchlist_;
        json["risk_params"] = risk_params_;
        json["auto_trade_enabled"] = auto_trade_enabled_;
        json["created_at"] = created_at_.toFormattedString(false);
        json["updated_at"] = updated_at_.toFormattedString(false);
        return json;
    }

    void UserSettings::fromJson(const Json::Value& json) {
        if (json.isMember("id")) {
            id_ = json["id"].asInt64();
        }

        if (!json.isMember("user_id") || !json.isMember("exchange_name")) {
            throw std::invalid_argument("Missing required fields in UserSettings JSON");
        }

        user_id_ = json["user_id"].asInt64();
        exchange_name_ = json["exchange_name"].asString();
        
        if (json.isMember("api_credentials")) {
            api_credentials_ = json["api_credentials"];
        }

        if (json.isMember("strategy_params")) {
            strategy_params_ = json["strategy_params"];
        }

        if (json.isMember("watchlist")) {
            watchlist_ = json["watchlist"];
        }

        if (json.isMember("risk_params")) {
            risk_params_ = json["risk_params"];
        }

        if (json.isMember("auto_trade_enabled")) {
            auto_trade_enabled_ = json["auto_trade_enabled"].asBool();
        }
        
        if (json.isMember("created_at")) {
            created_at_ = trantor::Date::fromDbString(json["created_at"].asString());
        }
        
        if (json.isMember("updated_at")) {
            updated_at_ = trantor::Date::fromDbString(json["updated_at"].asString());
        }
    }

    UserSettings UserSettings::fromDbRow(const drogon::orm::Row& row) {
        UserSettings settings;
        settings.setId(row["id"].as<int64_t>());
        settings.setUserId(row["user_id"].as<int64_t>());
        settings.setExchangeName(row["exchange_name"].as<std::string>());
        settings.setApiCredentials(utils::JsonUtils::parseJson(row["api_credentials"].as<std::string>()));
        settings.setStrategyParams(utils::JsonUtils::parseJson(row["strategy_params"].as<std::string>()));
        settings.setWatchlist(utils::JsonUtils::parseJson(row["watchlist"].as<std::string>()));
        settings.setRiskParams(utils::JsonUtils::parseJson(row["risk_params"].as<std::string>()));
        settings.setAutoTradeEnabled(row["auto_trade_enabled"].as<bool>());
        settings.setCreatedAt(trantor::Date::fromDbString(row["created_at"].as<std::string>()));
        settings.setUpdatedAt(trantor::Date::fromDbString(row["updated_at"].as<std::string>()));
        return settings;
    }

    std::vector<UserSettings> UserSettings::fromDbResult(const drogon::orm::Result& result) {
        std::vector<UserSettings> settingsList;
        settingsList.reserve(result.size());
        
        for (const auto& row : result) {
            settingsList.push_back(fromDbRow(row));
        }
        
        return settingsList;
    }

} // namespace models