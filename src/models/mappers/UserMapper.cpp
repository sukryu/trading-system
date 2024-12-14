#include "models/mappers/UserMapper.h"
#include <stdexcept>

namespace models {
    namespace mappers {

        UserMapper& UserMapper::getInstance() {
            static UserMapper instance;
            return instance;
        }

        User UserMapper::insert(const User& user) {
            const auto sql = 
                "INSERT INTO users (email, username, password_hash, is_active, last_login_at) "
                "VALUES ($1, $2, $3, $4, $5) RETURNING *";
                
            auto result = getDbClient()->execSqlSync(
                sql,
                user.getEmail(),
                user.getUsername(),
                user.getPasswordHash(),
                user.isActive(),
                user.getLastLoginAt().toFormattedString(false)
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert user");
            }

            return User::fromDbRow(result[0]);
        }

        User UserMapper::insert(const User& user, Transaction& trans) {
            const auto sql = 
                "INSERT INTO users (email, username, password_hash, is_active, last_login_at) "
                "VALUES ($1, $2, $3, $4, $5) RETURNING *";
                
            auto result = trans.execSqlSync(
                sql,
                user.getEmail(),
                user.getUsername(),
                user.getPasswordHash(),
                user.isActive(),
                user.getLastLoginAt().toFormattedString(false)
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert user in transaction");
            }

            return User::fromDbRow(result[0]);
        }

        User UserMapper::findById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM users WHERE id = $1",
                id
            );

            if (result.empty()) {
                throw std::runtime_error("User not found");
            }

            return User::fromDbRow(result[0]);
        }

        std::vector<User> UserMapper::findAll() {
            auto result = getDbClient()->execSqlSync("SELECT * FROM users");
            return User::fromDbResult(result);
        }

        std::vector<User> UserMapper::findAll(Transaction& trans) {
            auto result = trans.execSqlSync("SELECT * FROM users");
            return User::fromDbResult(result);
        }

        std::vector<User> UserMapper::findByCriteria(const std::string& whereClause) {
            std::string sql = "SELECT * FROM users";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return User::fromDbResult(result);
        }

        std::vector<User> UserMapper::findByCriteria(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT * FROM users";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return User::fromDbResult(result);
        }

        void UserMapper::update(const User& user) {
            const auto sql =
                "UPDATE users SET email = $1, username = $2, password_hash = $3, "
                "is_active = $4, last_login_at = $5 WHERE id = $6";

            auto result = getDbClient()->execSqlSync(
                sql,
                user.getEmail(),
                user.getUsername(),
                user.getPasswordHash(),
                user.isActive(),
                user.getLastLoginAt().toFormattedString(false),
                user.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User not found for update");
            }
        }

        void UserMapper::update(const User& user, Transaction& trans) {
            const auto sql =
                "UPDATE users SET email = $1, username = $2, password_hash = $3, "
                "is_active = $4, last_login_at = $5 WHERE id = $6";

            auto result = trans.execSqlSync(
                sql,
                user.getEmail(),
                user.getUsername(),
                user.getPasswordHash(),
                user.isActive(),
                user.getLastLoginAt().toFormattedString(false),
                user.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User not found for update in transaction");
            }
        }

        void UserMapper::deleteById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "DELETE FROM users WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User not found for deletion");
            }
        }

        void UserMapper::deleteById(int64_t id, Transaction& trans) {
            auto result = trans.execSqlSync(
                "DELETE FROM users WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User not found for deletion in transaction");
            }
        }

        size_t UserMapper::count(const std::string& whereClause) {
            std::string sql = "SELECT COUNT(*) FROM users";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        size_t UserMapper::count(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT COUNT(*) FROM users";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        std::vector<User> UserMapper::findWithPaging(size_t limit, size_t offset) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM users ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return User::fromDbResult(result);
        }

        std::vector<User> UserMapper::findWithPaging(size_t limit, size_t offset, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM users ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return User::fromDbResult(result);
        }

        std::optional<User> UserMapper::findByEmail(const std::string& email) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM users WHERE email = $1",
                email
            );

            if (result.empty()) {
                return std::nullopt;
            }

            return std::make_optional(User::fromDbRow(result[0]));
        }

        std::optional<User> UserMapper::findByUsername(const std::string& username) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM users WHERE username = $1",
                username
            );

            if (result.empty()) {
                return std::nullopt;
            }

            return std::make_optional(User::fromDbRow(result[0]));
        }

        void UserMapper::updateLastLoginTime(int64_t userId, const trantor::Date& loginTime) {
            auto result = getDbClient()->execSqlSync(
                "UPDATE users SET last_login_at = $1 WHERE id = $2",
                loginTime.toFormattedString(false),
                userId
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User not found for last login update");
            }
        }

        void UserMapper::updatePassword(int64_t userId, const std::string& newPasswordHash) {
            auto result = getDbClient()->execSqlSync(
                "UPDATE users SET password_hash = $1 WHERE id = $2",
                newPasswordHash,
                userId
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("User not found for password update");
            }
        }

        std::vector<User> UserMapper::findActiveUsers() {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM users WHERE is_active = true ORDER BY id"
            );
            return User::fromDbResult(result);
        }

    } // namespace mappers
} // namespace models