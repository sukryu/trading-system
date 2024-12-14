#include "models/mappers/MarketDataMapper.h"
#include <stdexcept>

namespace models {
    namespace mappers {

        MarketDataMapper& MarketDataMapper::getInstance() {
            static MarketDataMapper instance;
            return instance;
        }

        MarketData MarketDataMapper::insert(const MarketData& marketData, Transaction& transaction) {
            const auto sql = 
                "INSERT INTO market_data (symbol, price, volume, timestamp, source) "
                "VALUES ($1, $2, $3, $4, $5) RETURNING *";
            
            auto result = transaction.execSqlSync(
                sql,
                marketData.getSymbol(),
                marketData.getPrice(),
                marketData.getVolume(),
                marketData.getTimestamp().toFormattedString(false),
                marketData.getSource()
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert market data");
            }

            return MarketData::fromDbRow(result[0]);
        }

        MarketData MarketDataMapper::insert(const MarketData& marketData) {
            const auto sql = 
                "INSERT INTO market_data (symbol, price, volume, timestamp, source) "
                "VALUES ($1, $2, $3, $4, $5) RETURNING *";
            
            auto result = getDbClient()->execSqlSync(
                sql,
                marketData.getSymbol(),
                marketData.getPrice(),
                marketData.getVolume(),
                marketData.getTimestamp().toFormattedString(false),
                marketData.getSource()
            );

            if (result.empty()) {
                throw std::runtime_error("Failed to insert market data");
            }

            return MarketData::fromDbRow(result[0]);
        }

        MarketData MarketDataMapper::findById(int64_t id) {
            const auto sql = "SELECT * FROM market_data WHERE id = $1";
            
            auto result = getDbClient()->execSqlSync(sql, id);
            if (result.empty()) {
                throw std::runtime_error("Market data not found with id: " + std::to_string(id));
            }

            return MarketData::fromDbRow(result[0]);
        }

        std::vector<MarketData> MarketDataMapper::findWithPaging(size_t limit, size_t offset) {
            const auto sql = "SELECT * FROM market_data ORDER BY timestamp DESC LIMIT $1 OFFSET $2";
            auto result = getDbClient()->execSqlSync(sql, limit, offset);
            return MarketData::fromDbResult(result);
        }

        size_t MarketDataMapper::count() {
            const auto sql = "SELECT COUNT(*) as count FROM market_data";
            auto result = getDbClient()->execSqlSync(sql);
            return result[0]["count"].as<size_t>();
        }

        void MarketDataMapper::update(const MarketData& marketData, Transaction& transaction) {
            const auto sql = 
                "UPDATE market_data SET symbol = $1, price = $2, volume = $3, "
                "timestamp = $4, source = $5 WHERE id = $6";
            
            auto result = transaction.execSqlSync(
                sql,
                marketData.getSymbol(),
                marketData.getPrice(),
                marketData.getVolume(),
                marketData.getTimestamp().toFormattedString(false),
                marketData.getSource(),
                marketData.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Market data not found for update, id: " + 
                    std::to_string(marketData.getId()));
            }
        }

        void MarketDataMapper::update(const MarketData& marketData) {
            const auto sql = 
                "UPDATE market_data SET symbol = $1, price = $2, volume = $3, "
                "timestamp = $4, source = $5 WHERE id = $6";
            
            auto result = getDbClient()->execSqlSync(
                sql,
                marketData.getSymbol(),
                marketData.getPrice(),
                marketData.getVolume(),
                marketData.getTimestamp().toFormattedString(false),
                marketData.getSource(),
                marketData.getId()
            );

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Market data not found for update, id: " + 
                    std::to_string(marketData.getId()));
            }
        }

        void MarketDataMapper::deleteById(int64_t id, Transaction& transaction) {
            const auto sql = "DELETE FROM market_data WHERE id = $1";
            auto result = transaction.execSqlSync(sql, id);

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Market data not found for deletion, id: " + std::to_string(id));
            }
        }

        void MarketDataMapper::deleteById(int64_t id) {
            const auto sql = "DELETE FROM market_data WHERE id = $1";
            auto result = getDbClient()->execSqlSync(sql, id);

            if (result.affectedRows() == 0) {
                throw std::runtime_error("Market data not found for deletion, id: " + std::to_string(id));
            }
        }

        MarketData MarketDataMapper::findLatestBySymbol(const std::string& symbol) {
            const auto sql = 
                "SELECT * FROM market_data WHERE symbol = $1 "
                "ORDER BY timestamp DESC LIMIT 1";
            
            auto result = getDbClient()->execSqlSync(sql, symbol);
            if (result.empty()) {
                throw std::runtime_error("No market data found for symbol: " + symbol);
            }

            return MarketData::fromDbRow(result[0]);
        }

        std::vector<MarketData> MarketDataMapper::findBySymbolWithLimit(
            const std::string& symbol,
            size_t limit
        ) {
            const auto sql = 
                "SELECT * FROM market_data WHERE symbol = $1 "
                "ORDER BY timestamp DESC LIMIT $2";
            
            auto result = getDbClient()->execSqlSync(sql, symbol, limit);
            return MarketData::fromDbResult(result);
        }

        std::vector<MarketData> MarketDataMapper::findBySymbolAndTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end,
            Transaction& transaction
        ) {
            const auto sql = 
                "SELECT * FROM market_data "
                "WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3 "
                "ORDER BY timestamp";
            
            auto result = transaction.execSqlSync(
                sql,
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );
            return MarketData::fromDbResult(result);
        }

        std::vector<MarketData> MarketDataMapper::findBySymbolAndTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) {
            const auto sql = 
                "SELECT * FROM market_data "
                "WHERE symbol = $1 AND timestamp BETWEEN $2 AND $3 "
                "ORDER BY timestamp";
            
            auto result = getDbClient()->execSqlSync(
                sql,
                symbol,
                start.toFormattedString(false),
                end.toFormattedString(false)
            );
            return MarketData::fromDbResult(result);
        }

        std::vector<std::string> MarketDataMapper::getActiveSymbols(
            const trantor::Date& since,
            size_t minDataPoints
        ) {
            const auto sql = 
                "SELECT DISTINCT symbol FROM market_data "
                "WHERE timestamp >= $1 "
                "GROUP BY symbol "
                "HAVING COUNT(*) >= $2";
            
            auto result = getDbClient()->execSqlSync(
                sql,
                since.toFormattedString(false),
                minDataPoints
            );

            std::vector<std::string> symbols;
            symbols.reserve(result.size());
            
            for (const auto& row : result) {
                symbols.push_back(row["symbol"].as<std::string>());
            }
            
            return symbols;
        }

    } // namespace mappers
} // namespace models