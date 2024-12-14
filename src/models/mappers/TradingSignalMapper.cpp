#include "models/mappers/TradingSignalMapper.h"
#include "utils/JsonUtils.h"
#include <stdexcept>
#include <sstream>

namespace models {
    namespace mappers {

        TradingSignalMapper& TradingSignalMapper::getInstance() {
            static TradingSignalMapper instance;
            return instance;
        }

        TradingSignal TradingSignalMapper::insert(const TradingSignal& signal) {
            const auto sql = 
                "INSERT INTO trading_signals (symbol, signal_type, price, quantity, "
                "strategy_name, confidence, parameters, timestamp) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8) RETURNING *";
                
            auto result = getDbClient()->execSqlSync(
                sql,
                signal.getSymbol(),
                signal.getSignalType(),
                signal.getPrice(),
                signal.getQuantity(),
                signal.getStrategyName(),
                signal.getConfidence(),
                utils::JsonUtils::toJsonString(signal.getParameters()),
                signal.getTimestamp().toFormattedString(false)
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert trading signal");
            }

            return TradingSignal::fromDbRow(result[0]);
        }

        TradingSignal TradingSignalMapper::insert(const TradingSignal& signal, Transaction& trans) {
            const auto sql = 
                "INSERT INTO trading_signals (symbol, signal_type, price, quantity, "
                "strategy_name, confidence, parameters, timestamp) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8) RETURNING *";
                
            auto result = trans.execSqlSync(
                sql,
                signal.getSymbol(),
                signal.getSignalType(),
                signal.getPrice(),
                signal.getQuantity(),
                signal.getStrategyName(),
                signal.getConfidence(),
                utils::JsonUtils::toJsonString(signal.getParameters()),
                signal.getTimestamp().toFormattedString(false)
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert trading signal in transaction");
            }

            return TradingSignal::fromDbRow(result[0]);
        }

        TradingSignal TradingSignalMapper::findById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trading_signals WHERE id = $1",
                id
            );

            if (result.empty()) {
                throw std::runtime_error("Trading signal not found");
            }

            return TradingSignal::fromDbRow(result[0]);
        }

        std::vector<TradingSignal> TradingSignalMapper::findAll() {
            auto result = getDbClient()->execSqlSync("SELECT * FROM trading_signals");
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findAll(Transaction& trans) {
            auto result = trans.execSqlSync("SELECT * FROM trading_signals");
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findByCriteria(const std::string& whereClause) {
            std::string sql = "SELECT * FROM trading_signals";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findByCriteria(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT * FROM trading_signals";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return TradingSignal::fromDbResult(result);
        }

        void TradingSignalMapper::update(const TradingSignal& signal) {
            const auto sql =
                "UPDATE trading_signals SET symbol = $1, signal_type = $2, price = $3, "
                "quantity = $4, strategy_name = $5, confidence = $6, parameters = $7, "
                "timestamp = $8 WHERE id = $9";

            auto result = getDbClient()->execSqlSync(
                sql,
                signal.getSymbol(),
                signal.getSignalType(),
                signal.getPrice(),
                signal.getQuantity(),
                signal.getStrategyName(),
                signal.getConfidence(),
                utils::JsonUtils::toJsonString(signal.getParameters()),
                signal.getTimestamp().toFormattedString(false),
                signal.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Trading signal not found for update");
            }
        }

        void TradingSignalMapper::update(const TradingSignal& signal, Transaction& trans) {
            const auto sql =
                "UPDATE trading_signals SET symbol = $1, signal_type = $2, price = $3, "
                "quantity = $4, strategy_name = $5, confidence = $6, parameters = $7, "
                "timestamp = $8 WHERE id = $9";

            auto result = trans.execSqlSync(
                sql,
                signal.getSymbol(),
                signal.getSignalType(),
                signal.getPrice(),
                signal.getQuantity(),
                signal.getStrategyName(),
                signal.getConfidence(),
                utils::JsonUtils::toJsonString(signal.getParameters()),
                signal.getTimestamp().toFormattedString(false),
                signal.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Trading signal not found for update in transaction");
            }
        }

        void TradingSignalMapper::deleteById(int64_t id) {
            auto result = getDbClient()->execSqlSync(
                "DELETE FROM trading_signals WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Trading signal not found for deletion");
            }
        }

        void TradingSignalMapper::deleteById(int64_t id, Transaction& trans) {
            auto result = trans.execSqlSync(
                "DELETE FROM trading_signals WHERE id = $1",
                id
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Trading signal not found for deletion in transaction");
            }
        }

        size_t TradingSignalMapper::count(const std::string& whereClause) {
            std::string sql = "SELECT COUNT(*) FROM trading_signals";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = getDbClient()->execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        size_t TradingSignalMapper::count(const std::string& whereClause, Transaction& trans) {
            std::string sql = "SELECT COUNT(*) FROM trading_signals";
            if (!whereClause.empty()) {
                sql += " WHERE " + whereClause;
            }
            
            auto result = trans.execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        std::vector<TradingSignal> TradingSignalMapper::findWithPaging(size_t limit, size_t offset) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trading_signals ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findWithPaging(size_t limit, size_t offset, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM trading_signals ORDER BY id LIMIT $1 OFFSET $2",
                limit,
                offset
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findBySymbol(const std::string& symbol) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trading_signals WHERE symbol = $1 ORDER BY timestamp DESC",
                symbol
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findBySymbol(const std::string& symbol, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM trading_signals WHERE symbol = $1 ORDER BY timestamp DESC",
                symbol
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findByStrategyName(const std::string& strategyName) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trading_signals WHERE strategy_name = $1 ORDER BY timestamp DESC",
                strategyName
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findByStrategyName(const std::string& strategyName, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT * FROM trading_signals WHERE strategy_name = $1 ORDER BY timestamp DESC",
                strategyName
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findByTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) {
            auto result = getDbClient()->execSqlSync(
                "SELECT * FROM trading_signals WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3 "
                "ORDER BY timestamp",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findByTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end,
            Transaction& trans
        ) {
            auto result = trans.execSqlSync(
                "SELECT * FROM trading_signals WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3 "
                "ORDER BY timestamp",
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findPendingSignals(const std::string& symbol) {
            auto result = getDbClient()->execSqlSync(
                "SELECT ts.* FROM trading_signals ts "
                "LEFT JOIN orders o ON ts.id = o.signal_id "
                "WHERE ts.symbol = $1 AND o.id IS NULL "
                "ORDER BY ts.timestamp DESC",
                symbol
            );
            return TradingSignal::fromDbResult(result);
        }

        std::vector<TradingSignal> TradingSignalMapper::findPendingSignals(const std::string& symbol, Transaction& trans) {
            auto result = trans.execSqlSync(
                "SELECT ts.* FROM trading_signals ts "
                "LEFT JOIN orders o ON ts.id = o.signal_id "
                "WHERE ts.symbol = $1 AND o.id IS NULL "
                "ORDER BY ts.timestamp DESC",
                symbol
            );
            return TradingSignal::fromDbResult(result);
        }

    } // namespace mappers
} // namespace models
