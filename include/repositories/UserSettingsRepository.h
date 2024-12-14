#pragma once

#include "repositories/BaseRepository.h"
#include "models/UserSettings.h"
#include "models/mappers/UserSettingsMapper.h"
#include <string>
#include <optional>
#include <vector>
#include <trantor/utils/Date.h>

namespace repositories {

    class UserSettingsRepository : public BaseRepository<models::UserSettings> {
    public:
        using TransactionPtr = std::shared_ptr<drogon::orm::Transaction>;

        static UserSettingsRepository& getInstance();

        // BaseRepository 구현
        models::UserSettings save(const models::UserSettings& settings) override;
        std::optional<models::UserSettings> findById(int64_t id) const override;
        bool deleteById(int64_t id) override;

        PaginationResult findAll(size_t page, size_t pageSize) const override;
        std::vector<models::UserSettings> findByTimeRange(
            const trantor::Date& start,
            const trantor::Date& end
        ) const override;

        // UserSettings 전용 메서드
        std::vector<models::UserSettings> findByUserId(int64_t userId) const;
        std::optional<models::UserSettings> findByUserAndExchange(
            int64_t userId, 
            const std::string& exchangeName
        ) const;
        std::vector<models::UserSettings> findAutoTradeEnabled() const;

        // 설정 업데이트 메서드
        void updateAutoTradeStatus(int64_t settingsId, bool enabled);
        void updateApiCredentials(int64_t settingsId, const Json::Value& credentials);
        void updateStrategyParams(int64_t settingsId, const Json::Value& params);
        void updateWatchlist(int64_t settingsId, const Json::Value& watchlist);
        void updateRiskParams(int64_t settingsId, const Json::Value& riskParams);

        // 벌크 작업
        void saveBatch(const std::vector<models::UserSettings>& settingsList);

    private:
        UserSettingsRepository() = default;
        ~UserSettingsRepository() override = default;
        UserSettingsRepository(const UserSettingsRepository&) = delete;
        UserSettingsRepository& operator=(const UserSettingsRepository&) = delete;

        models::mappers::UserSettingsMapper& mapper_{models::mappers::UserSettingsMapper::getInstance()};
    };

} // namespace repositories