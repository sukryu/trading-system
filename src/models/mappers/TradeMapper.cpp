#include "models/mappers/TradeMapper.h"
#include <stdexcept>
#include <sstream>

namespace models {
    namespace mappers {

        TradeMapper& TradeMapper::getInstance() {
            static TradeMapper instance;
            return instance;
        }

        Trade TradeMapper::insert(const Trade& trade) {
            const auto sql = 
                "INSERT INTO trades (trade_id, order_id, symbol, side, quantity, "
                "price, commission, commission_asset, timestamp) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9) RETURNING *";
                
            auto result = getDbClient()->execSqlSync(
                sql,
                trade.getTradeId(),
                trade.getOrderId(),
                trade.getSymbol(),
                trade.getSide(),
                trade.getQuantity(),
                trade.getPrice(),
                trade.getCommission(),
                trade.getCommissionAsset(),
                trade.getTimestamp().toFormattedString(false)
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert trade");
            }

            return Trade::fromDbRow(result[0]);
        }

        Trade TradeMapper::insert(const Trade& trade, Transaction& trans) {
            const auto sql = 
                "INSERT INTO trades (trade_id, order_id, symbol, side, quantity, "
                "price, commission, commission_asset, timestamp) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9) RETURNING *";
                
            auto result = trans.execSqlSync(
                sql,
                trade.getTradeId(),
                trade.getOrderId(),
                trade.getSymbol(),
                trade.getSide(),
                trade.getQuantity(),
                trade.getPrice(),
                trade.getCommission(),
                trade.getCommissionAsset(),
                trade.getTimestamp().toFormattedString(false)
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert trade in transaction");
            }

            return Trade::fromDbRow(result[0]);
        }

        Trade TradeMapper::findById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trades WHERE id = $1",
                id
            );

            if (result.empty()) {
                throw std::runtime_error("Trade not found");
            }

            return Trade::fromDbRow(result[0]);
        }

        std::vector<Trade> TradeMapper::findAll() {
            auto result = getDbClient()->execSqlSync("SELECT * FROM trades");
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findAll(Transaction& trans) {
            auto result = trans.execSqlSync("SELECT * FROM trades");
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findByCriteria(const std::string& whereClause) {
            std::string sql = "SELECT * FROM trades";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findByCriteria(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT * FROM trades";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return Trade::fromDbResult(result);
        }

        void TradeMapper::update(const Trade& trade) {
            const auto sql =
                "UPDATE trades SET trade_id = $1, order_id = $2, symbol = $3, "
                "side = $4, quantity = $5, price = $6, commission = $7, "
                "commission_asset = $8, timestamp = $9 WHERE id = $10";

            auto result = getDbClient()->execSqlSync(
                sql,
                trade.getTradeId(),
                trade.getOrderId(),
                trade.getSymbol(),
                trade.getSide(),
                trade.getQuantity(),
                trade.getPrice(),
                trade.getCommission(),
                trade.getCommissionAsset(),
                trade.getTimestamp().toFormattedString(false),
                trade.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Trade not found for update");
            }
        }

        void TradeMapper::update(const Trade& trade, Transaction& trans) {
            const auto sql =
                "UPDATE trades SET trade_id = $1, order_id = $2, symbol = $3, "
                "side = $4, quantity = $5, price = $6, commission = $7, "
                "commission_asset = $8, timestamp = $9 WHERE id = $10";

            auto result = trans.execSqlSync(
                sql,
                trade.getTradeId(),
                trade.getOrderId(),
                trade.getSymbol(),
                trade.getSide(),
                trade.getQuantity(),
                trade.getPrice(),
                trade.getCommission(),
                trade.getCommissionAsset(),
                trade.getTimestamp().toFormattedString(false),
                trade.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Trade not found for update in transaction");
            }
        }

        void TradeMapper::deleteById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "DELETE FROM trades WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Trade not found for deletion");
            }
        }

        void TradeMapper::deleteById(int64_t id, Transaction& trans) {
            auto result = trans.execSqlSync(
                "DELETE FROM trades WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Trade not found for deletion in transaction");
            }
        }

        size_t TradeMapper::count(const std::string& whereClause) {
            std::string sql = "SELECT COUNT(*) FROM trades";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        size_t TradeMapper::count(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT COUNT(*) FROM trades";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        std::vector<Trade> TradeMapper::findWithPaging(size_t limit, size_t offset) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trades ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findWithPaging(size_t limit, size_t offset, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM trades ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findBySymbol(const std::string& symbol) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trades WHERE symbol = $1 ORDER BY timestamp DESC",
                symbol
            );
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findBySymbol(const std::string& symbol, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM trades WHERE symbol = $1 ORDER BY timestamp DESC",
                symbol
            );
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findByOrderId(int64_t orderId) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trades WHERE order_id = $1 ORDER BY timestamp",
                orderId
            );
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findByOrderId(int64_t orderId, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM trades WHERE order_id = $1 ORDER BY timestamp",
                orderId
            );
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findByTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trades WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3 "
                "ORDER BY timestamp",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );
            return Trade::fromDbResult(result);
        }

        std::vector<Trade> TradeMapper::findByTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end,
            Transaction& trans
        ) {
            auto result = trans.execSqlSync(
                "SELECT * FROM trades WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3 "
                "ORDER BY timestamp",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );
            return Trade::fromDbResult(result);
        }

        double TradeMapper::getSymbolTotalVolume(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) {
            auto result = getDbClient()->execSqlSync(
                "SELECT SUM(quantity) as total_volume FROM trades "
                "WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );

            if (result.empty() || result[0]["total_volume"].isNull()) {
                return 0.0;
            }

            return result[0]["total_volume"].as<double>();
        }

        double TradeMapper::getSymbolTotalVolume(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end,
            Transaction& trans
        ) {
            auto result = trans.execSqlSync(
                "SELECT SUM(quantity) as total_volume FROM trades "
                "WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );

            if (result.empty() || result[0]["total_volume"].isNull()) {
                return 0.0;
            }

            return result[0]["total_volume"].as<double>();
        }

        double TradeMapper::getSymbolAveragePrice(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) {
            auto result = getDbClient()->execSqlSync(
                "SELECT AVG(price) as avg_price FROM trades "
                "WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );

            if (result.empty() || result[0]["avg_price"].isNull()) {
                return 0.0;
            }

            return result[0]["avg_price"].as<double>();
        }

        double TradeMapper::getSymbolAveragePrice(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end,
            Transaction& trans
        ) {
            auto result = trans.execSqlSync(
                "SELECT AVG(price) as avg_price FROM trades "
                "WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );

            if (result.empty() || result[0]["avg_price"].isNull()) {
                return 0.0;
            }

            return result[0]["avg_price"].as<double>();
        }

    } // namespace mappers
} // namespace models
