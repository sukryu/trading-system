#pragma once

#include <drogon/drogon.h>
#include "models/mappers/BaseMapper.h"
#include "models/TradingSignal.h"

namespace models {
    namespace mappers {

        class TradingSignalMapper : public BaseMapper<TradingSignal> {
        public:
            using Transaction = drogon::orm::Transaction;

            static TradingSignalMapper& getInstance();

            // 기본 CRUD
            TradingSignal insert(const TradingSignal& signal) override;
            TradingSignal insert(const TradingSignal& signal, Transaction& trans);

            TradingSignal findById(int64_t id) override;

            std::vector<TradingSignal> findAll() override;
            std::vector<TradingSignal> findAll(Transaction& trans);

            std::vector<TradingSignal> findByCriteria(const std::string& whereClause) override;
            std::vector<TradingSignal> findByCriteria(const std::string& whereClause, Transaction& trans);

            void update(const TradingSignal& signal) override;
            void update(const TradingSignal& signal, Transaction& trans);

            void deleteById(int64_t id) override;
            void deleteById(int64_t id, Transaction& trans);

            size_t count(const std::string& whereClause = "") override;
            size_t count(const std::string& whereClause, Transaction& trans);

            std::vector<TradingSignal> findWithPaging(size_t limit, size_t offset) override;
            std::vector<TradingSignal> findWithPaging(size_t limit, size_t offset, Transaction& trans);

            // TradingSignal 전용 메서드
            std::vector<TradingSignal> findBySymbol(const std::string& symbol);
            std::vector<TradingSignal> findBySymbol(const std::string& symbol, Transaction& trans);

            std::vector<TradingSignal> findByStrategyName(const std::string& strategyName);
            std::vector<TradingSignal> findByStrategyName(const std::string& strategyName, Transaction& trans);

            std::vector<TradingSignal> findByTimeRange(
                const std::string& symbol,
                const trantor::Date& start,
                const trantor::Date& end
            );
            std::vector<TradingSignal> findByTimeRange(
                const std::string& symbol,
                const trantor::Date& start,
                const trantor::Date& end,
                Transaction& trans
            );

            std::vector<TradingSignal> findPendingSignals(const std::string& symbol);
            std::vector<TradingSignal> findPendingSignals(const std::string& symbol, Transaction& trans);

        private:
            TradingSignalMapper() = default;
            ~TradingSignalMapper() override = default;
            TradingSignalMapper(const TradingSignalMapper&) = delete;
            TradingSignalMapper& operator=(const TradingSignalMapper&) = delete;

            drogon::orm::DbClientPtr getDbClient() const {
                return drogon::app().getDbClient();
            }
        };

    } // namespace mappers
} // namespace models
