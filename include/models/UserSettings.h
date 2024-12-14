#pragma once

#include "models/BaseModel.h"
#include <string>
#include <json/json.h>
#include <trantor/utils/Date.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Result.h>
#include <vector>

namespace models {

    class UserSettings : public BaseModel {
    public:
        UserSettings() = default;
        ~UserSettings() override = default;

        // Getters
        int64_t getId() const { return id_; }
        int64_t getUserId() const { return user_id_; }
        const std::string& getExchangeName() const { return exchange_name_; }
        const Json::Value& getApiCredentials() const { return api_credentials_; }
        const Json::Value& getStrategyParams() const { return strategy_params_; }
        const Json::Value& getWatchlist() const { return watchlist_; }
        const Json::Value& getRiskParams() const { return risk_params_; }
        bool isAutoTradeEnabled() const { return auto_trade_enabled_; }
        const trantor::Date& getCreatedAt() const { return created_at_; }
        const trantor::Date& getUpdatedAt() const { return updated_at_; }

        // Setters
        void setId(int64_t id) { id_ = id; }
        void setUserId(int64_t user_id) { user_id_ = user_id; }
        void setExchangeName(const std::string& exchange_name) { exchange_name_ = exchange_name; }
        void setApiCredentials(const Json::Value& api_credentials) { api_credentials_ = api_credentials; }
        void setStrategyParams(const Json::Value& strategy_params) { strategy_params_ = strategy_params; }
        void setWatchlist(const Json::Value& watchlist) { watchlist_ = watchlist; }
        void setRiskParams(const Json::Value& risk_params) { risk_params_ = risk_params; }
        void setAutoTradeEnabled(bool enabled) { auto_trade_enabled_ = enabled; }
        void setCreatedAt(const trantor::Date& created_at) { created_at_ = created_at; }
        void setUpdatedAt(const trantor::Date& updated_at) { updated_at_ = updated_at; }

        // JSON conversion
        Json::Value toJson() const override;
        void fromJson(const Json::Value& json) override;

        // DB conversion
        static UserSettings fromDbRow(const drogon::orm::Row& row);
        static std::vector<UserSettings> fromDbResult(const drogon::orm::Result& result);

    private:
        int64_t id_{0};
        int64_t user_id_{0};
        std::string exchange_name_;
        Json::Value api_credentials_;    // {api_key, api_secret, additional_params}
        Json::Value strategy_params_;    // {strategy_type, ma_period, rsi_period, etc}
        Json::Value watchlist_;         // ["BTC/USDT", "ETH/USDT", ...]
        Json::Value risk_params_;       // {max_position_size, stop_loss_pct, take_profit_pct}
        bool auto_trade_enabled_{false};
        trantor::Date created_at_;
        trantor::Date updated_at_;
    };

} // namespace models