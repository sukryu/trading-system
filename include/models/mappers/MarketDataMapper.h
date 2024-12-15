#pragma once

#include <drogon/drogon.h>
#include <trantor/utils/Date.h>
#include "models/MarketData.h"
#include <memory>
#include <string>
#include <vector>

namespace models {
    namespace mappers {

        class MarketDataMapper {
        public:
            using Transaction = drogon::orm::Transaction;
            using DbClientPtr = drogon::orm::DbClientPtr;

            static MarketDataMapper& getInstance();

            // 기본 CRUD
            std::shared_ptr<MarketData> insert(
                const std::shared_ptr<MarketData>& marketData,
                Transaction& transaction
            );
            std::shared_ptr<MarketData> insert(const std::shared_ptr<MarketData>& marketData);

            std::shared_ptr<MarketData> findById(int64_t id);
            std::vector<std::shared_ptr<MarketData>> findWithPaging(size_t limit, size_t offset);
            size_t count();

            void update(const std::shared_ptr<MarketData>& marketData, Transaction& transaction);
            void update(const std::shared_ptr<MarketData>& marketData);

            void deleteById(int64_t id, Transaction& transaction);
            void deleteById(int64_t id);

            // 특화된 쿼리 메서드
            std::shared_ptr<MarketData> findLatestBySymbol(const std::string& symbol);
            
            std::vector<std::shared_ptr<MarketData>> findBySymbolWithLimit(
                const std::string& symbol,
                size_t limit
            );

            std::vector<std::shared_ptr<MarketData>> findBySymbolAndTimeRange(
                const std::string& symbol,
                const trantor::Date& start,
                const trantor::Date& end,
                Transaction& transaction
            );
            std::vector<std::shared_ptr<MarketData>> findBySymbolAndTimeRange(
                const std::string& symbol,
                const trantor::Date& start,
                const trantor::Date& end
            );

            std::vector<std::string> getActiveSymbols(
                const trantor::Date& since,
                size_t minDataPoints = 1
            );

        private:
            MarketDataMapper() = default;
            ~MarketDataMapper() = default;
            MarketDataMapper(const MarketDataMapper&) = delete;
            MarketDataMapper& operator=(const MarketDataMapper&) = delete;

            DbClientPtr getDbClient() const {
                return drogon::app().getDbClient();
            }
        };

    } // namespace mappers
} // namespace models
