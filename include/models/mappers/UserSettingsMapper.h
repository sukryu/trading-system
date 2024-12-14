#pragma once

#include <drogon/drogon.h>
#include "models/mappers/BaseMapper.h"
#include "models/UserSettings.h"

namespace models {
    namespace mappers {

        class UserSettingsMapper : public BaseMapper<UserSettings> {
        public:
            using Transaction = drogon::orm::Transaction;

            static UserSettingsMapper& getInstance();

            // 기본 CRUD
            UserSettings insert(const UserSettings& settings) override;
            UserSettings insert(const UserSettings& settings, Transaction& trans);

            UserSettings findById(int64_t id) override;

            std::vector<UserSettings> findAll() override;
            std::vector<UserSettings> findAll(Transaction& trans);

            std::vector<UserSettings> findByCriteria(const std::string& whereClause) override;
            std::vector<UserSettings> findByCriteria(const std::string& whereClause, Transaction& trans);

            void update(const UserSettings& settings) override;
            void update(const UserSettings& settings, Transaction& trans);

            void deleteById(int64_t id) override;
            void deleteById(int64_t id, Transaction& trans);

            size_t count(const std::string& whereClause = "") override;
            size_t count(const std::string& whereClause, Transaction& trans);

            std::vector<UserSettings> findWithPaging(size_t limit, size_t offset) override;
            std::vector<UserSettings> findWithPaging(size_t limit, size_t offset, Transaction& trans);

            // UserSettings 전용 메서드
            std::vector<UserSettings> findByUserId(int64_t userId);
            std::optional<UserSettings> findByUserAndExchange(int64_t userId, const std::string& exchangeName);
            std::vector<UserSettings> findAutoTradeEnabled();
            void updateAutoTradeStatus(int64_t settingsId, bool enabled);
            void updateApiCredentials(int64_t settingsId, const Json::Value& credentials);
            void updateStrategyParams(int64_t settingsId, const Json::Value& params);
            void updateWatchlist(int64_t settingsId, const Json::Value& watchlist);
            void updateRiskParams(int64_t settingsId, const Json::Value& riskParams);

        private:
            UserSettingsMapper() = default;
            ~UserSettingsMapper() override = default;
            UserSettingsMapper(const UserSettingsMapper&) = delete;
            UserSettingsMapper& operator=(const UserSettingsMapper&) = delete;

            drogon::orm::DbClientPtr getDbClient() const {
                return drogon::app().getDbClient();
            }
        };

    } // namespace mappers
} // namespace models