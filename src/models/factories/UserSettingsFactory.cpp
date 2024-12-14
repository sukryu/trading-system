#include "models/factories/UserSettingsFactory.h"
#include <stdexcept>

namespace models {
    namespace factories {

        const UserSettingsFactory& UserSettingsFactory::getInstance() {
            static UserSettingsFactory instance;
            return instance;
        }

        std::shared_ptr<BaseModel> UserSettingsFactory::createFromDbRow(const drogon::orm::Row& row) const {
            try {
                auto settings = std::make_shared<UserSettings>();
                *settings = UserSettings::fromDbRow(row);
                return settings;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in UserSettingsFactory::createFromDbRow: ") + e.what());
            }
        }

        std::vector<std::shared_ptr<BaseModel>> UserSettingsFactory::createFromDbResult(const drogon::orm::Result& result) const {
            std::vector<std::shared_ptr<BaseModel>> settingsList;
            settingsList.reserve(result.size());
            
            for (const auto& row : result) {
                settingsList.push_back(createFromDbRow(row));
            }
            
            return settingsList;
        }

        std::shared_ptr<BaseModel> UserSettingsFactory::createFromJson(const Json::Value& json) const {
            try {
                auto settings = std::make_shared<UserSettings>();
                settings->fromJson(json);
                return settings;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in UserSettingsFactory::createFromJson: ") + e.what());
            }
        }

        std::shared_ptr<BaseModel> UserSettingsFactory::create() const {
            return std::make_shared<UserSettings>();
        }

    } // namespace factories
} // namespace models