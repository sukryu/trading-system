#include "repositories/MarketDataRepository.h"
#include <stdexcept>
#include <sstream>
#include <cmath>

namespace repositories {

    MarketDataRepository& MarketDataRepository::getInstance() {
        static MarketDataRepository instance;
        return instance;
    }

    models::MarketData MarketDataRepository::save(const models::MarketData& marketData) {
        // id가 0이면 새 레코드 삽입, 아니면 업데이트
        if (marketData.getId() == 0) {
            return mapper_.insert(marketData);
        } else {
            mapper_.update(marketData);
            return marketData;
        }
    }

    std::optional<models::MarketData> MarketDataRepository::findById(int64_t id) const {
        try {
            return std::make_optional(mapper_.findById(id));
        } catch (const std::runtime_error&) {
            return std::nullopt;
        }
    }

    bool MarketDataRepository::deleteById(int64_t id) {
        try {
            mapper_.deleteById(id);
            return true;
        } catch (const std::runtime_error&) {
            return false;
        }
    }

    BaseRepository<models::MarketData>::PaginationResult 
    MarketDataRepository::findAll(size_t page, size_t pageSize) const {
        auto totalCount = mapper_.count();
        auto items = mapper_.findWithPaging(pageSize, page * pageSize);

        PaginationResult result;
        result.items = std::move(items);
        result.totalCount = totalCount;
        result.pageSize = pageSize;
        result.currentPage = page;
        result.totalPages = (totalCount + pageSize - 1) / pageSize;

        return result;
    }

    std::vector<models::MarketData> MarketDataRepository::findByTimeRange(
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        // 현재 Mapper는 특정 symbol 없이 전체 MarketData를 시간범위로 조회하는 메서드가 없음.
        // 필요하다면 Mapper를 수정하거나, 여기서 예외를 던짐.
        throw std::runtime_error("findByTimeRange without symbol is not supported by the current mapper");
    }

    std::optional<models::MarketData> MarketDataRepository::findLatestBySymbol(const std::string& symbol) const {
        try {
            return std::make_optional(mapper_.findLatestBySymbol(symbol));
        } catch (const std::runtime_error&) {
            return std::nullopt;
        }
    }

    std::vector<models::MarketData> MarketDataRepository::findBySymbol(
        const std::string& symbol,
        size_t limit
    ) const {
        return mapper_.findBySymbolWithLimit(symbol, limit);
    }

    std::vector<models::MarketData> MarketDataRepository::findBySymbolAndTimeRange(
        const std::string& symbol,
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        return mapper_.findBySymbolAndTimeRange(symbol, start, end);
    }

    double MarketDataRepository::getLatestPrice(const std::string& symbol) const {
        auto latestData = findLatestBySymbol(symbol);
        if (!latestData) {
            throw std::runtime_error("No price data available for symbol: " + symbol);
        }
        return latestData->getPrice();
    }

    bool MarketDataRepository::hasPriceChangeExceededThreshold(
        const std::string& symbol,
        double threshold,
        size_t timeWindowMinutes
    ) const {
        auto now = trantor::Date::now();
        auto windowStart = now.after(-static_cast<double>(timeWindowMinutes) * 60);

        auto recentData = findBySymbolAndTimeRange(symbol, windowStart, now);
        if (recentData.size() < 2) {
            return false;  // 데이터 부족
        }

        double latestPrice = recentData.front().getPrice();
        double oldestPrice = recentData.back().getPrice();

        double priceChange = std::fabs((latestPrice - oldestPrice) / oldestPrice) * 100.0;
        return priceChange >= threshold;
    }

    std::vector<std::string> MarketDataRepository::getActiveSymbols(
        const trantor::Date& since,
        size_t minDataPoints
    ) const {
        return mapper_.getActiveSymbols(since, minDataPoints);
    }

    void MarketDataRepository::saveBatch(const std::vector<models::MarketData>& marketDataList) {
        this->executeInTransaction([this, &marketDataList](const TransactionPtr& transPtr) {
            for (const auto& data : marketDataList) {
                if (data.getId() == 0) {
                    mapper_.insert(data, *transPtr);
                } else {
                    mapper_.update(data, *transPtr);
                }
            }
        });
    }

    void MarketDataRepository::invalidateCache(const std::string& symbol) {
        // TODO: 캐시 무효화 로직 구현
    }

    void MarketDataRepository::warmupCache(const std::string& symbol) {
        // TODO: 캐시 프리로딩 로직 구현
    }

    std::string MarketDataRepository::createCacheKey(
        const std::string& symbol,
        const std::string& suffix
    ) const {
        return "market_data:" + symbol + (suffix.empty() ? "" : ":" + suffix);
    }

} // namespace repositories
