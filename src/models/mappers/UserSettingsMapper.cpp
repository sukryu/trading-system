#include "models/mappers/UserSettingsMapper.h"
#include "utils/JsonUtils.h"
#include <stdexcept>

namespace models {
    namespace mappers {

        UserSettingsMapper& UserSettingsMapper::getInstance() {
            static UserSettingsMapper instance;
            return instance;
        }

        UserSettings UserSettingsMapper::insert(const UserSettings& settings) {
            const auto sql = 
                "INSERT INTO user_settings (user_id, exchange_name, api_credentials, "
                "strategy_params, watchlist, risk_params, auto_trade_enabled) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING *";
                
            auto result = getDbClient()->execSqlSync(
                sql,
                settings.getUserId(),
                settings.getExchangeName(),
                utils::JsonUtils::toJsonString(settings.getApiCredentials()),
                utils::JsonUtils::toJsonString(settings.getStrategyParams()),
                utils::JsonUtils::toJsonString(settings.getWatchlist()),
                utils::JsonUtils::toJsonString(settings.getRiskParams()),
                settings.isAutoTradeEnabled()
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert user settings");
            }

            return UserSettings::fromDbRow(result[0]);
        }

        UserSettings UserSettingsMapper::insert(const UserSettings& settings, Transaction& trans) {
            const auto sql = 
                "INSERT INTO user_settings (user_id, exchange_name, api_credentials, "
                "strategy_params, watchlist, risk_params, auto_trade_enabled) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING *";
                
            auto result = trans.execSqlSync(
                sql,
                settings.getUserId(),
                settings.getExchangeName(),
                utils::JsonUtils::toJsonString(settings.getApiCredentials()),
                utils::JsonUtils::toJsonString(settings.getStrategyParams()),
                utils::JsonUtils::toJsonString(settings.getWatchlist()),
                utils::JsonUtils::toJsonString(settings.getRiskParams()),
                settings.isAutoTradeEnabled()
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert user settings in transaction");
            }

            return UserSettings::fromDbRow(result[0]);
        }

        UserSettings UserSettingsMapper::findById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM user_settings WHERE id = $1",
                id
            );

            if (result.empty()) {
                throw std::runtime_error("User settings not found");
            }

            return UserSettings::fromDbRow(result[0]);
        }

        std::vector<UserSettings> UserSettingsMapper::findAll() {
            auto result = getDbClient()->execSqlSync("SELECT * FROM user_settings");
            return UserSettings::fromDbResult(result);
        }

        std::vector<UserSettings> UserSettingsMapper::findAll(Transaction& trans) {
            auto result = trans.execSqlSync("SELECT * FROM user_settings");
            return UserSettings::fromDbResult(result);
        }

        std::vector<UserSettings> UserSettingsMapper::findByCriteria(const std::string& whereClause) {
            std::string sql = "SELECT * FROM user_settings";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return UserSettings::fromDbResult(result);
        }

        std::vector<UserSettings> UserSettingsMapper::findByCriteria(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT * FROM user_settings";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return UserSettings::fromDbResult(result);
        }

        void UserSettingsMapper::update(const UserSettings& settings) {
            const auto sql =
                "UPDATE user_settings SET exchange_name = $1, api_credentials = $2, "
                "strategy_params = $3, watchlist = $4, risk_params = $5, "
                "auto_trade_enabled = $6 WHERE id = $7";

            auto result = getDbClient()->execSqlSync(
                sql,
                settings.getExchangeName(),
                utils::JsonUtils::toJsonString(settings.getApiCredentials()),
                utils::JsonUtils::toJsonString(settings.getStrategyParams()),
                utils::JsonUtils::toJsonString(settings.getWatchlist()),
                utils::JsonUtils::toJsonString(settings.getRiskParams()),
                settings.isAutoTradeEnabled(),
                settings.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for update");
            }
        }

        void UserSettingsMapper::update(const UserSettings& settings, Transaction& trans) {
            const auto sql =
                "UPDATE user_settings SET exchange_name = $1, api_credentials = $2, "
                "strategy_params = $3, watchlist = $4, risk_params = $5, "
                "auto_trade_enabled = $6 WHERE id = $7";

            auto result = trans.execSqlSync(
                sql,
                settings.getExchangeName(),
                utils::JsonUtils::toJsonString(settings.getApiCredentials()),
                utils::JsonUtils::toJsonString(settings.getStrategyParams()),
                utils::JsonUtils::toJsonString(settings.getWatchlist()),
                utils::JsonUtils::toJsonString(settings.getRiskParams()),
                settings.isAutoTradeEnabled(),
                settings.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for update in transaction");
            }
        }

        void UserSettingsMapper::deleteById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "DELETE FROM user_settings WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for deletion");
            }
        }

        void UserSettingsMapper::deleteById(int64_t id, Transaction& trans) {
            auto result = trans.execSqlSync(
                "DELETE FROM user_settings WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for deletion in transaction");
            }
        }

        size_t UserSettingsMapper::count(const std::string& whereClause) {
            std::string sql = "SELECT COUNT(*) FROM user_settings";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        size_t UserSettingsMapper::count(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT COUNT(*) FROM user_settings";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        std::vector<UserSettings> UserSettingsMapper::findWithPaging(size_t limit, size_t offset) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM user_settings ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return UserSettings::fromDbResult(result);
        }

        std::vector<UserSettings> UserSettingsMapper::findWithPaging(size_t limit, size_t offset, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM user_settings ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return UserSettings::fromDbResult(result);
        }

        std::vector<UserSettings> UserSettingsMapper::findByUserId(int64_t userId) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM user_settings WHERE user_id = $1 ORDER BY exchange_name",
                userId
            );
            return UserSettings::fromDbResult(result);
        }

        std::optional<UserSettings> UserSettingsMapper::findByUserAndExchange(
            int64_t userId, 
            const std::string& exchangeName
        ) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM user_settings WHERE user_id = $1 AND exchange_name = $2",
                userId,
                exchangeName
            );

            if (result.empty()) {
                return std::nullopt;
            }

            return std::make_optional(UserSettings::fromDbRow(result[0]));
        }

        std::vector<UserSettings> UserSettingsMapper::findAutoTradeEnabled() {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM user_settings WHERE auto_trade_enabled = true ORDER BY user_id"
            );
            return UserSettings::fromDbResult(result);
        }

        void UserSettingsMapper::updateAutoTradeStatus(int64_t settingsId, bool enabled) {
            auto result = getDbClient()->execSqlSync(
                "UPDATE user_settings SET auto_trade_enabled = $1 WHERE id = $2",
                enabled,
                settingsId
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for auto trade status update");
            }
        }

        void UserSettingsMapper::updateApiCredentials(int64_t settingsId, const Json::Value& credentials) {
            auto result = getDbClient()->execSqlSync(
                "UPDATE user_settings SET api_credentials = $1 WHERE id = $2",
                utils::JsonUtils::toJsonString(credentials),
                settingsId
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for API credentials update");
            }
        }

        void UserSettingsMapper::updateStrategyParams(int64_t settingsId, const Json::Value& params) {
            auto result = getDbClient()->execSqlSync(
                "UPDATE user_settings SET strategy_params = $1 WHERE id = $2",
                utils::JsonUtils::toJsonString(params),
                settingsId
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for strategy params update");
            }
        }

        void UserSettingsMapper::updateWatchlist(int64_t settingsId, const Json::Value& watchlist) {
            auto result = getDbClient()->execSqlSync(
                "UPDATE user_settings SET watchlist = $1 WHERE id = $2",
                utils::JsonUtils::toJsonString(watchlist),
                settingsId
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for watchlist update");
            }
        }

        void UserSettingsMapper::updateRiskParams(int64_t settingsId, const Json::Value& riskParams) {
            auto result = getDbClient()->execSqlSync(
                "UPDATE user_settings SET risk_params = $1 WHERE id = $2",
                utils::JsonUtils::toJsonString(riskParams),
                settingsId
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User settings not found for risk params update");
            }
        }

    } // namespace mappers
} // namespace models