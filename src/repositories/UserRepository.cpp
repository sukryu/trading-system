#include "repositories/UserRepository.h"
#include <stdexcept>
#include <cmath>

namespace repositories {

    UserRepository& UserRepository::getInstance() {
        static UserRepository instance;
        return instance;
    }

    models::User UserRepository::save(const models::User& user) {
        if (user.getId() == 0) {
            return mapper_.insert(user);
        } else {
            mapper_.update(user);
            return user;
        }
    }

    std::optional<models::User> UserRepository::findById(int64_t id) const {
        try {
            return std::make_optional(mapper_.findById(id));
        } catch (const std::runtime_error&) {
            return std::nullopt;
        }
    }

    bool UserRepository::deleteById(int64_t id) {
        try {
            mapper_.deleteById(id);
            return true;
        } catch (const std::runtime_error&) {
            return false;
        }
    }

    BaseRepository<models::User>::PaginationResult 
    UserRepository::findAll(size_t page, size_t pageSize) const {
        auto totalCount = mapper_.count();
        auto items = mapper_.findWithPaging(pageSize, page * pageSize);

        PaginationResult result;
        result.items = std::move(items);
        result.totalCount = totalCount;
        result.pageSize = pageSize;
        result.currentPage = page;
        result.totalPages = (totalCount + pageSize - 1) / pageSize;

        return result;
    }

    std::vector<models::User> UserRepository::findByTimeRange(
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        throw std::runtime_error("Time range search not applicable for User entity");
    }

    std::optional<models::User> UserRepository::findByEmail(const std::string& email) const {
        return mapper_.findByEmail(email);
    }

    std::optional<models::User> UserRepository::findByUsername(const std::string& username) const {
        return mapper_.findByUsername(username);
    }

    std::vector<models::User> UserRepository::findActiveUsers() const {
        return mapper_.findActiveUsers();
    }

    void UserRepository::updateLastLoginTime(int64_t userId, const trantor::Date& loginTime) {
        mapper_.updateLastLoginTime(userId, loginTime);
    }

    void UserRepository::updatePassword(int64_t userId, const std::string& newPasswordHash) {
        mapper_.updatePassword(userId, newPasswordHash);
    }

    void UserRepository::setUserActive(int64_t userId, bool active) {
        this->executeInTransaction([this, userId, active](const TransactionPtr& transPtr) {
            auto user = mapper_.findById(userId);
            user.setIsActive(active);
            mapper_.update(user, *transPtr);
        });
    }

    void UserRepository::saveBatch(const std::vector<models::User>& userList) {
        this->executeInTransaction([this, &userList](const TransactionPtr& transPtr) {
            for (const auto& user : userList) {
                if (user.getId() == 0) {
                    mapper_.insert(user, *transPtr);
                } else {
                    mapper_.update(user, *transPtr);
                }
            }
        });
    }

} // namespace repositories