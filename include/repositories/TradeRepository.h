#pragma once

#include "repositories/BaseRepository.h"
#include "models/Trade.h"
#include "models/mappers/TradeMapper.h"
#include <string>
#include <optional>
#include <vector>
#include <trantor/utils/Date.h>

namespace repositories {

    class TradeRepository : public BaseRepository<models::Trade> {
    public:
        using TransactionPtr = std::shared_ptr<drogon::orm::Transaction>;

        static TradeRepository& getInstance();

        // BaseRepository 구현
        models::Trade save(const models::Trade& trade) override;
        std::optional<models::Trade> findById(int64_t id) const override;
        bool deleteById(int64_t id) override;

        PaginationResult findAll(size_t page, size_t pageSize) const override;
        std::vector<models::Trade> findByTimeRange(
            const trantor::Date& start,
            const trantor::Date& end
        ) const override;

        // Trade 전용 메서드
        std::vector<models::Trade> findBySymbol(const std::string& symbol, size_t limit = 100) const;
        std::vector<models::Trade> findByOrderId(int64_t orderId, size_t limit = 100) const;

        std::vector<models::Trade> findBySymbolAndTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) const;

        double getSymbolTotalVolume(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) const;

        double getSymbolAveragePrice(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) const;

        // 벌크 작업
        void saveBatch(const std::vector<models::Trade>& tradeList);

    private:
        TradeRepository() = default;
        ~TradeRepository() override = default;
        TradeRepository(const TradeRepository&) = delete;
        TradeRepository& operator=(const TradeRepository&) = delete;

        models::mappers::TradeMapper& mapper_{models::mappers::TradeMapper::getInstance()};
    };

} // namespace repositories
