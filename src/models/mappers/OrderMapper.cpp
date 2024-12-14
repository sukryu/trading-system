#include "models/mappers/OrderMapper.h"
#include <stdexcept>
#include <sstream>

namespace models {
    namespace mappers {

        OrderMapper& OrderMapper::getInstance() {
            static OrderMapper instance;
            return instance;
        }

        // insert
        Order OrderMapper::insert(const Order& order) {
            const auto sql = 
                "INSERT INTO orders (order_id, symbol, order_type, side, quantity, "
                "price, status, signal_id, filled_quantity, filled_price, "
                "error_message, timestamp) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12) RETURNING *";
                
            auto result = getDbClient()->execSqlSync(
                sql,
                order.getOrderId(),
                order.getSymbol(),
                order.getOrderType(),
                order.getSide(),
                order.getQuantity(),
                order.getPrice(),
                order.getStatus(),
                order.getSignalId(),
                order.getFilledQuantity(),
                order.getFilledPrice(),
                order.getErrorMessage(),
                order.getTimestamp().toFormattedString(false)
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert order");
            }

            return Order::fromDbRow(result[0]);
        }

        Order OrderMapper::insert(const Order& order, Transaction& trans) {
            const auto sql = 
                "INSERT INTO orders (order_id, symbol, order_type, side, quantity, "
                "price, status, signal_id, filled_quantity, filled_price, "
                "error_message, timestamp) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12) RETURNING *";

            auto result = trans.execSqlSync(
                sql,
                order.getOrderId(),
                order.getSymbol(),
                order.getOrderType(),
                order.getSide(),
                order.getQuantity(),
                order.getPrice(),
                order.getStatus(),
                order.getSignalId(),
                order.getFilledQuantity(),
                order.getFilledPrice(),
                order.getErrorMessage(),
                order.getTimestamp().toFormattedString(false)
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert order in transaction");
            }

            return Order::fromDbRow(result[0]);
        }

        // findById
        Order OrderMapper::findById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM orders WHERE id = $1",
                id
            );

            if (result.empty()) {
                throw std::runtime_error("Order not found");
            }

            return Order::fromDbRow(result[0]);
        }

        // findAll
        std::vector<Order> OrderMapper::findAll() {
            auto result = getDbClient()->execSqlSync("SELECT * FROM orders");
            return Order::fromDbResult(result);
        }

        // findByCriteria
        std::vector<Order> OrderMapper::findByCriteria(const std::string& whereClause) {
            std::string sql = "SELECT * FROM orders";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return Order::fromDbResult(result);
        }

        std::vector<Order> OrderMapper::findByCriteria(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT * FROM orders";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return Order::fromDbResult(result);
        }

        // update
        void OrderMapper::update(const Order& order) {
            const auto sql =
                "UPDATE orders SET order_id = $1, symbol = $2, order_type = $3, "
                "side = $4, quantity = $5, price = $6, status = $7, signal_id = $8, "
                "filled_quantity = $9, filled_price = $10, error_message = $11, "
                "timestamp = $12, updated_at = CURRENT_TIMESTAMP WHERE id = $13";

            auto result = getDbClient()->execSqlSync(
                sql,
                order.getOrderId(),
                order.getSymbol(),
                order.getOrderType(),
                order.getSide(),
                order.getQuantity(),
                order.getPrice(),
                order.getStatus(),
                order.getSignalId(),
                order.getFilledQuantity(),
                order.getFilledPrice(),
                order.getErrorMessage(),
                order.getTimestamp().toFormattedString(false),
                order.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Order not found for update");
            }
        }

        void OrderMapper::update(const Order& order, Transaction& trans) {
            const auto sql =
                "UPDATE orders SET order_id = $1, symbol = $2, order_type = $3, "
                "side = $4, quantity = $5, price = $6, status = $7, signal_id = $8, "
                "filled_quantity = $9, filled_price = $10, error_message = $11, "
                "timestamp = $12, updated_at = CURRENT_TIMESTAMP WHERE id = $13";

            auto result = trans.execSqlSync(
                sql,
                order.getOrderId(),
                order.getSymbol(),
                order.getOrderType(),
                order.getSide(),
                order.getQuantity(),
                order.getPrice(),
                order.getStatus(),
                order.getSignalId(),
                order.getFilledQuantity(),
                order.getFilledPrice(),
                order.getErrorMessage(),
                order.getTimestamp().toFormattedString(false),
                order.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Order not found for update in transaction");
            }
        }

        // deleteById
        void OrderMapper::deleteById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "DELETE FROM orders WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Order not found for deletion");
            }
        }

        void OrderMapper::deleteById(int64_t id, Transaction& trans) {
            auto result = trans.execSqlSync(
                "DELETE FROM orders WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Order not found for deletion in transaction");
            }
        }

        // count
        size_t OrderMapper::count(const std::string& whereClause) {
            std::string sql = "SELECT COUNT(*) FROM orders";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        // findWithPaging
        std::vector<Order> OrderMapper::findWithPaging(size_t limit, size_t offset) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM orders ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return Order::fromDbResult(result);
        }

        // findBySymbol
        std::vector<Order> OrderMapper::findBySymbol(const std::string& symbol) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM orders WHERE symbol = $1 ORDER BY timestamp DESC",
                symbol
            );
            return Order::fromDbResult(result);
        }

        std::vector<Order> OrderMapper::findBySymbol(const std::string& symbol, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM orders WHERE symbol = $1 ORDER BY timestamp DESC",
                symbol
            );
            return Order::fromDbResult(result);
        }

        // findByStatus
        std::vector<Order> OrderMapper::findByStatus(const std::string& status) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM orders WHERE status = $1 ORDER BY timestamp DESC",
                status
            );
            return Order::fromDbResult(result);
        }

        std::vector<Order> OrderMapper::findByStatus(const std::string& status, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM orders WHERE status = $1 ORDER BY timestamp DESC",
                status
            );
            return Order::fromDbResult(result);
        }

        // findBySignalId
        std::vector<Order> OrderMapper::findBySignalId(int64_t signalId) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM orders WHERE signal_id = $1 ORDER BY timestamp DESC",
                signalId
            );
            return Order::fromDbResult(result);
        }

        std::vector<Order> OrderMapper::findBySignalId(int64_t signalId, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM orders WHERE signal_id = $1 ORDER BY timestamp DESC",
                signalId
            );
            return Order::fromDbResult(result);
        }

        // findByTimeRange
        std::vector<Order> OrderMapper::findByTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM orders WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3 "
                "ORDER BY timestamp",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );
            return Order::fromDbResult(result);
        }

        std::vector<Order> OrderMapper::findByTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end,
            Transaction& trans
        ) {
            auto result = trans.execSqlSync(
                "SELECT * FROM orders WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3 "
                "ORDER BY timestamp",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );
            return Order::fromDbResult(result);
        }

        // findPendingOrders
        std::vector<Order> OrderMapper::findPendingOrders(const std::string& symbol) {
            std::string sql = "SELECT * FROM orders WHERE status = 'PENDING'";
            if (!symbol.empty()) {
                sql += " AND symbol = $1";
                auto result = getDbClient()->execSqlSync(sql, symbol);
                return Order::fromDbResult(result);
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return Order::fromDbResult(result);
        }

        std::vector<Order> OrderMapper::findPendingOrders(const std::string& symbol, Transaction& trans) {
            std::string sql = "SELECT * FROM orders WHERE status = 'PENDING'";
            if (!symbol.empty()) {
                sql += " AND symbol = $1";
                auto result = trans.execSqlSync(sql, symbol);
                return Order::fromDbResult(result);
            }
            
            auto result = trans.execSqlSync(sql);
            return Order::fromDbResult(result);
        }

        // updateOrderStatus
        void OrderMapper::updateOrderStatus(int64_t id, const std::string& status, 
                                            double filledQuantity, double filledPrice) {
            auto result = getDbClient()->execSqlSync(
                "UPDATE orders SET status = $1, filled_quantity = $2, filled_price = $3, "
                "updated_at = CURRENT_TIMESTAMP WHERE id = $4",
                status,
                filledQuantity,
                filledPrice,
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Order not found for status update");
            }
        }

        void OrderMapper::updateOrderStatus(int64_t id, const std::string& status, 
                                            double filledQuantity, double filledPrice,
                                            Transaction& trans) {
            auto result = trans.execSqlSync(
                "UPDATE orders SET status = $1, filled_quantity = $2, filled_price = $3, "
                "updated_at = CURRENT_TIMESTAMP WHERE id = $4",
                status,
                filledQuantity,
                filledPrice,
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Order not found for status update in transaction");
            }
        }

    } // namespace mappers
} // namespace models
