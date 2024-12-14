#pragma once

#include "repositories/BaseRepository.h"
#include "models/TradingSignal.h"
#include "models/mappers/TradingSignalMapper.h"
#include <string>
#include <optional>
#include <vector>
#include <trantor/utils/Date.h>

namespace repositories {

    class TradingSignalRepository : public BaseRepository<models::TradingSignal> {
    public:
        using TransactionPtr = std::shared_ptr<drogon::orm::Transaction>;

        static TradingSignalRepository& getInstance();

        // BaseRepository 구현
        models::TradingSignal save(const models::TradingSignal& signal) override;
        std::optional<models::TradingSignal> findById(int64_t id) const override;
        bool deleteById(int64_t id) override;

        PaginationResult findAll(size_t page, size_t pageSize) const override;
        std::vector<models::TradingSignal> findByTimeRange(
            const trantor::Date& start,
            const trantor::Date& end
        ) const override;

        // TradingSignal 전용 메서드
        std::vector<models::TradingSignal> findBySymbol(const std::string& symbol, size_t limit = 100) const;
        std::vector<models::TradingSignal> findByStrategyName(const std::string& strategyName, size_t limit = 100) const;
        std::vector<models::TradingSignal> findPendingSignals(const std::string& symbol) const;

        // 심볼과 시간 범위로 필터
        std::vector<models::TradingSignal> findBySymbolAndTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) const;

        // 벌크 작업
        void saveBatch(const std::vector<models::TradingSignal>& signalList);

    private:
        TradingSignalRepository() = default;
        ~TradingSignalRepository() override = default;
        TradingSignalRepository(const TradingSignalRepository&) = delete;
        TradingSignalRepository& operator=(const TradingSignalRepository&) = delete;

        models::mappers::TradingSignalMapper& mapper_{models::mappers::TradingSignalMapper::getInstance()};
    };

} // namespace repositories
