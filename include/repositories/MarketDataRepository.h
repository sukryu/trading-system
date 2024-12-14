#pragma once

#include "repositories/BaseRepository.h"
#include "models/MarketData.h"
#include "models/mappers/MarketDataMapper.h"
#include <string>
#include <optional>
#include <vector>

namespace repositories {

    class MarketDataRepository : public BaseRepository<models::MarketData> {
    public:
        using TransactionPtr = std::shared_ptr<drogon::orm::Transaction>;

        static MarketDataRepository& getInstance();

        // BaseRepository 구현
        models::MarketData save(const models::MarketData& marketData) override;
        std::optional<models::MarketData> findById(int64_t id) const override;
        bool deleteById(int64_t id) override;

        PaginationResult findAll(size_t page, size_t pageSize) const override;
        std::vector<models::MarketData> findByTimeRange(
            const trantor::Date& start,
            const trantor::Date& end
        ) const override;

        // MarketData 전용 메서드
        std::optional<models::MarketData> findLatestBySymbol(const std::string& symbol) const;
        std::vector<models::MarketData> findBySymbol(
            const std::string& symbol,
            size_t limit = 100
        ) const;
        std::vector<models::MarketData> findBySymbolAndTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) const;
        double getLatestPrice(const std::string& symbol) const;
        bool hasPriceChangeExceededThreshold(
            const std::string& symbol,
            double threshold,
            size_t timeWindowMinutes = 5
        ) const;
        std::vector<std::string> getActiveSymbols(
            const trantor::Date& since,
            size_t minDataPoints = 1
        ) const;

        // 벌크 작업
        void saveBatch(const std::vector<models::MarketData>& marketDataList);

        // 캐시 관련 (TODO)
        void invalidateCache(const std::string& symbol);
        void warmupCache(const std::string& symbol);

    private:
        MarketDataRepository() = default;
        ~MarketDataRepository() override = default;
        MarketDataRepository(const MarketDataRepository&) = delete;
        MarketDataRepository& operator=(const MarketDataRepository&) = delete;

        models::mappers::MarketDataMapper& mapper_{models::mappers::MarketDataMapper::getInstance()};

        // 캐시 키 생성 헬퍼
        std::string createCacheKey(const std::string& symbol, const std::string& suffix = "") const;
    };

} // namespace repositories
