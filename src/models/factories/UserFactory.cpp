#include "models/factories/UserFactory.h"
#include <stdexcept>

namespace models {
    namespace factories {

        const UserFactory& UserFactory::getInstance() {
            static UserFactory instance;
            return instance;
        }

        std::shared_ptr<BaseModel> UserFactory::createFromDbRow(const drogon::orm::Row& row) const {
            try {
                auto user = std::make_shared<User>();
                *user = User::fromDbRow(row);
                return user;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in UserFactory::createFromDbRow: ") + e.what());
            }
        }

        std::vector<std::shared_ptr<BaseModel>> UserFactory::createFromDbResult(const drogon::orm::Result& result) const {
            std::vector<std::shared_ptr<BaseModel>> users;
            users.reserve(result.size());
            
            for (const auto& row : result) {
                users.push_back(createFromDbRow(row));
            }
            
            return users;
        }

        std::shared_ptr<BaseModel> UserFactory::createFromJson(const Json::Value& json) const {
            try {
                auto user = std::make_shared<User>();
                user->fromJson(json);
                return user;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in UserFactory::createFromJson: ") + e.what());
            }
        }

        std::shared_ptr<BaseModel> UserFactory::create() const {
            return std::make_shared<User>();
        }

    } // namespace factories
} // namespace models