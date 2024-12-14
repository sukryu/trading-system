#pragma once

#include "repositories/BaseRepository.h"
#include "models/User.h"
#include "models/mappers/UserMapper.h"
#include <string>
#include <optional>
#include <vector>
#include <trantor/utils/Date.h>

namespace repositories {

    class UserRepository : public BaseRepository<models::User> {
    public:
        using TransactionPtr = std::shared_ptr<drogon::orm::Transaction>;

        static UserRepository& getInstance();

        // BaseRepository 구현
        models::User save(const models::User& user) override;
        std::optional<models::User> findById(int64_t id) const override;
        bool deleteById(int64_t id) override;

        PaginationResult findAll(size_t page, size_t pageSize) const override;
        std::vector<models::User> findByTimeRange(
            const trantor::Date& start,
            const trantor::Date& end
        ) const override;

        // User 전용 메서드
        std::optional<models::User> findByEmail(const std::string& email) const;
        std::optional<models::User> findByUsername(const std::string& username) const;
        std::vector<models::User> findActiveUsers() const;
        
        // 인증 관련 메서드
        void updateLastLoginTime(int64_t userId, const trantor::Date& loginTime);
        void updatePassword(int64_t userId, const std::string& newPasswordHash);
        
        // 상태 관련 메서드
        void setUserActive(int64_t userId, bool active);

        // 벌크 작업
        void saveBatch(const std::vector<models::User>& userList);

    private:
        UserRepository() = default;
        ~UserRepository() override = default;
        UserRepository(const UserRepository&) = delete;
        UserRepository& operator=(const UserRepository&) = delete;

        models::mappers::UserMapper& mapper_{models::mappers::UserMapper::getInstance()};
    };

} // namespace repositories