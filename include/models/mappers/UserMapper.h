#pragma once

#include <drogon/drogon.h>
#include "models/mappers/BaseMapper.h"
#include "models/User.h"

namespace models {
    namespace mappers {

        class UserMapper : public BaseMapper<User> {
        public:
            using Transaction = drogon::orm::Transaction;

            static UserMapper& getInstance();

            // 기본 CRUD
            User insert(const User& user) override;
            User insert(const User& user, Transaction& trans);

            User findById(int64_t id) override;

            std::vector<User> findAll() override;
            std::vector<User> findAll(Transaction& trans);

            std::vector<User> findByCriteria(const std::string& whereClause) override;
            std::vector<User> findByCriteria(const std::string& whereClause, Transaction& trans);

            void update(const User& user) override;
            void update(const User& user, Transaction& trans);

            void deleteById(int64_t id) override;
            void deleteById(int64_t id, Transaction& trans);

            size_t count(const std::string& whereClause = "") override;
            size_t count(const std::string& whereClause, Transaction& trans);

            std::vector<User> findWithPaging(size_t limit, size_t offset) override;
            std::vector<User> findWithPaging(size_t limit, size_t offset, Transaction& trans);

            // User 전용 메서드
            std::optional<User> findByEmail(const std::string& email);
            std::optional<User> findByUsername(const std::string& username);
            void updateLastLoginTime(int64_t userId, const trantor::Date& loginTime);
            void updatePassword(int64_t userId, const std::string& newPasswordHash);
            std::vector<User> findActiveUsers();

        private:
            UserMapper() = default;
            ~UserMapper() override = default;
            UserMapper(const UserMapper&) = delete;
            UserMapper& operator=(const UserMapper&) = delete;

            drogon::orm::DbClientPtr getDbClient() const {
                return drogon::app().getDbClient();
            }
        };

    } // namespace mappers
} // namespace models