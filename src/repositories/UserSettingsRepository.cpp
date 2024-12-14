#include "repositories/UserSettingsRepository.h"
#include "secure/EncryptionManager.h"
#include "secure/SensitiveDataHandler.h"
#include <stdexcept>
#include <cmath>

namespace repositories {

    UserSettingsRepository& UserSettingsRepository::getInstance() {
        static UserSettingsRepository instance;
        return instance;
    }

    models::UserSettings UserSettingsRepository::save(const models::UserSettings& settings) {
        // 만약 settings에 credentials 필드(Json) 있고, apiKey/secret이 들어있다면 암호화
        Json::Value creds = settings.getApiCredentials(); // 가정: UserSettings 모델에 getCredentials() 존재
        if (creds.isMember("apiKey") || creds.isMember("secret")) {
            auto& enc = EncryptionManager::getInstance();
            if (creds.isMember("apiKey")) {
                std::string plainKey = creds["apiKey"].asString();
                creds["apiKey"] = enc.encrypt(plainKey);
            }
            if (creds.isMember("secret")) {
                std::string plainSecret = creds["secret"].asString();
                creds["secret"] = enc.encrypt(plainSecret);
            }
            // 암호화된 credentials를 다시 settings에 반영
            models::UserSettings modified = settings;
            modified.setApiCredentials(creds);
            if (modified.getId() == 0) {
                return mapper_.insert(modified);
            } else {
                mapper_.update(modified);
                return modified;
            }
        } else {
            // 민감정보 없으면 그대로
            if (settings.getId() == 0) {
                return mapper_.insert(settings);
            } else {
                mapper_.update(settings);
                return settings;
            }
        }
    }

    std::optional<models::UserSettings> UserSettingsRepository::findById(int64_t id) const {
        try {
            auto result = std::make_optional(mapper_.findById(id));
            // 조회 후 credentials 복호화
            if (result && result->getApiCredentials().isObject()) {
                auto creds = result->getApiCredentials();
                auto& enc = EncryptionManager::getInstance();
                if (creds.isMember("apiKey")) {
                    creds["apiKey"] = enc.decrypt(creds["apiKey"].asString());
                }
                if (creds.isMember("secret")) {
                    creds["secret"] = enc.decrypt(creds["secret"].asString());
                }
                models::UserSettings modified = *result;
                modified.setApiCredentials(creds);
                return modified;
            }
            return result;
        } catch (const std::runtime_error&) {
            return std::nullopt;
        }
    }

    bool UserSettingsRepository::deleteById(int64_t id) {
        try {
            mapper_.deleteById(id);
            return true;
        } catch (const std::runtime_error&) {
            return false;
        }
    }

    BaseRepository<models::UserSettings>::PaginationResult 
    UserSettingsRepository::findAll(size_t page, size_t pageSize) const {
        auto totalCount = mapper_.count();
        auto items = mapper_.findWithPaging(pageSize, page * pageSize);

        // items 내 각 UserSettings에 대해 credentials 복호화
        auto& enc = EncryptionManager::getInstance();
        for (auto &st : items) {
            auto creds = st.getApiCredentials();
            if (creds.isMember("apiKey")) {
                creds["apiKey"] = enc.decrypt(creds["apiKey"].asString());
            }
            if (creds.isMember("secret")) {
                creds["secret"] = enc.decrypt(creds["secret"].asString());
            }
            st.setApiCredentials(creds);
        }

        PaginationResult result;
        result.items = std::move(items);
        result.totalCount = totalCount;
        result.pageSize = pageSize;
        result.currentPage = page;
        result.totalPages = (totalCount + pageSize - 1) / pageSize;

        return result;
    }

    std::vector<models::UserSettings> UserSettingsRepository::findByTimeRange(
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        throw std::runtime_error("Time range search not applicable for UserSettings entity");
    }

    std::vector<models::UserSettings> UserSettingsRepository::findByUserId(int64_t userId) const {
        auto items = mapper_.findByUserId(userId);
        auto& enc = EncryptionManager::getInstance();
        for (auto &st : items) {
            auto creds = st.getApiCredentials();
            if (creds.isMember("apiKey")) {
                creds["apiKey"] = enc.decrypt(creds["apiKey"].asString());
            }
            if (creds.isMember("secret")) {
                creds["secret"] = enc.decrypt(creds["secret"].asString());
            }
            st.setApiCredentials(creds);
        }
        return items;
    }

    std::optional<models::UserSettings> UserSettingsRepository::findByUserAndExchange(
        int64_t userId, 
        const std::string& exchangeName
    ) const {
        auto result = mapper_.findByUserAndExchange(userId, exchangeName);
        if (result) {
            auto creds = result->getApiCredentials();
            auto& enc = EncryptionManager::getInstance();
            if (creds.isMember("apiKey")) {
                creds["apiKey"] = enc.decrypt(creds["apiKey"].asString());
            }
            if (creds.isMember("secret")) {
                creds["secret"] = enc.decrypt(creds["secret"].asString());
            }
            models::UserSettings modified = *result;
            modified.setApiCredentials(creds);
            return modified;
        }
        return std::nullopt;
    }

    std::vector<models::UserSettings> UserSettingsRepository::findAutoTradeEnabled() const {
        auto items = mapper_.findAutoTradeEnabled();
        auto& enc = EncryptionManager::getInstance();
        for (auto &st : items) {
            auto creds = st.getApiCredentials();
            if (creds.isMember("apiKey")) {
                creds["apiKey"] = enc.decrypt(creds["apiKey"].asString());
            }
            if (creds.isMember("secret")) {
                creds["secret"] = enc.decrypt(creds["secret"].asString());
            }
            st.setApiCredentials(creds);
        }
        return items;
    }

    void UserSettingsRepository::updateAutoTradeStatus(int64_t settingsId, bool enabled) {
        mapper_.updateAutoTradeStatus(settingsId, enabled);
    }

    void UserSettingsRepository::updateApiCredentials(int64_t settingsId, const Json::Value& credentials) {
        auto& enc = EncryptionManager::getInstance();
        Json::Value encCreds = credentials;
        if (encCreds.isMember("apiKey")) {
            encCreds["apiKey"] = enc.encrypt(encCreds["apiKey"].asString());
        }
        if (encCreds.isMember("secret")) {
            encCreds["secret"] = enc.encrypt(encCreds["secret"].asString());
        }
        mapper_.updateApiCredentials(settingsId, encCreds);
    }

    void UserSettingsRepository::updateStrategyParams(int64_t settingsId, const Json::Value& params) {
        mapper_.updateStrategyParams(settingsId, params);
    }

    void UserSettingsRepository::updateWatchlist(int64_t settingsId, const Json::Value& watchlist) {
        mapper_.updateWatchlist(settingsId, watchlist);
    }

    void UserSettingsRepository::updateRiskParams(int64_t settingsId, const Json::Value& riskParams) {
        mapper_.updateRiskParams(settingsId, riskParams);
    }

    void UserSettingsRepository::saveBatch(const std::vector<models::UserSettings>& settingsList) {
        this->executeInTransaction([this, &settingsList](const TransactionPtr& transPtr) {
            auto& enc = EncryptionManager::getInstance();
            for (auto settings : settingsList) {
                auto creds = settings.getApiCredentials();
                if (creds.isMember("apiKey")) {
                    creds["apiKey"] = enc.encrypt(creds["apiKey"].asString());
                }
                if (creds.isMember("secret")) {
                    creds["secret"] = enc.encrypt(creds["secret"].asString());
                }
                settings.setApiCredentials(creds);

                if (settings.getId() == 0) {
                    mapper_.insert(settings, *transPtr);
                } else {
                    mapper_.update(settings, *transPtr);
                }
            }
        });
    }
} // namespace repositories