#pragma once

#include <drogon/drogon.h>
#include "models/mappers/BaseMapper.h"
#include "models/Trade.h"

namespace models {
    namespace mappers {

        class TradeMapper : public BaseMapper<Trade> {
        public:
            using Transaction = drogon::orm::Transaction;

            static TradeMapper& getInstance();

            // 기본 CRUD
            Trade insert(const Trade& trade) override;
            Trade insert(const Trade& trade, Transaction& trans);

            Trade findById(int64_t id) override;

            std::vector<Trade> findAll() override;
            std::vector<Trade> findAll(Transaction& trans);

            std::vector<Trade> findByCriteria(const std::string& whereClause) override;
            std::vector<Trade> findByCriteria(const std::string& whereClause, Transaction& trans);

            void update(const Trade& trade) override;
            void update(const Trade& trade, Transaction& trans);

            void deleteById(int64_t id) override;
            void deleteById(int64_t id, Transaction& trans);

            size_t count(const std::string& whereClause = "") override;
            size_t count(const std::string& whereClause, Transaction& trans);

            std::vector<Trade> findWithPaging(size_t limit, size_t offset) override;
            std::vector<Trade> findWithPaging(size_t limit, size_t offset, Transaction& trans);

            // Trade 전용 메서드
            std::vector<Trade> findBySymbol(const std::string& symbol);
            std::vector<Trade> findBySymbol(const std::string& symbol, Transaction& trans);

            std::vector<Trade> findByOrderId(int64_t orderId);
            std::vector<Trade> findByOrderId(int64_t orderId, Transaction& trans);

            std::vector<Trade> findByTimeRange(
                const std::string& symbol,
                const trantor::Date& start,
                const trantor::Date& end
            );
            std::vector<Trade> findByTimeRange(
                const std::string& symbol,
                const trantor::Date& start,
                const trantor::Date& end,
                Transaction& trans
            );

            double getSymbolTotalVolume(const std::string& symbol, 
                                        const trantor::Date& start, 
                                        const trantor::Date& end);
            double getSymbolTotalVolume(const std::string& symbol, 
                                        const trantor::Date& start, 
                                        const trantor::Date& end,
                                        Transaction& trans);

            double getSymbolAveragePrice(const std::string& symbol, 
                                         const trantor::Date& start, 
                                         const trantor::Date& end);
            double getSymbolAveragePrice(const std::string& symbol, 
                                         const trantor::Date& start, 
                                         const trantor::Date& end,
                                         Transaction& trans);

        private:
            TradeMapper() = default;
            ~TradeMapper() override = default;
            TradeMapper(const TradeMapper&) = delete;
            TradeMapper& operator=(const TradeMapper&) = delete;

            drogon::orm::DbClientPtr getDbClient() const {
                return drogon::app().getDbClient();
            }
        };

    } // namespace mappers
} // namespace models
