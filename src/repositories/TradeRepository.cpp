#include "repositories/TradeRepository.h"
#include <stdexcept>
#include <cmath>

namespace repositories {

    TradeRepository& TradeRepository::getInstance() {
        static TradeRepository instance;
        return instance;
    }

    models::Trade TradeRepository::save(const models::Trade& trade) {
        if (trade.getId() == 0) {
            return mapper_.insert(trade);
        } else {
            mapper_.update(trade);
            return trade;
        }
    }

    std::optional<models::Trade> TradeRepository::findById(int64_t id) const {
        try {
            return std::make_optional(mapper_.findById(id));
        } catch (const std::runtime_error&) {
            return std::nullopt;
        }
    }

    bool TradeRepository::deleteById(int64_t id) {
        try {
            mapper_.deleteById(id);
            return true;
        } catch (const std::runtime_error&) {
            return false;
        }
    }

    BaseRepository<models::Trade>::PaginationResult
    TradeRepository::findAll(size_t page, size_t pageSize) const {
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

    std::vector<models::Trade> TradeRepository::findByTimeRange(
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        // 현재 TradeMapper는 전체 심볼에 대한 time range 조회가 별도로 없는 경우가 많음.
        // 필요하다면 TradeMapper에 전체 심볼 대상 time range 메서드 추가.
        // 여기서는 예외를 던지거나 symbol을 인자로 받는 메서드를 사용하도록 강제.
        throw std::runtime_error("findByTimeRange without symbol is not supported by the current mapper");
    }

    std::vector<models::Trade> TradeRepository::findBySymbol(
        const std::string& symbol,
        size_t limit
    ) const {
        // findBySymbolWithLimit 가 없으므로 limit 적용하려면 결과를 잘라야 함.
        // 여기서는 일단 TradeMapper의 findBySymbol 사용 후 상위 limit개만 반환.
        auto trades = mapper_.findBySymbol(symbol);
        if (trades.size() > limit) {
            trades.resize(limit);
        }
        return trades;
    }

    std::vector<models::Trade> TradeRepository::findByOrderId(
        int64_t orderId,
        size_t limit
    ) const {
        auto trades = mapper_.findByOrderId(orderId);
        if (trades.size() > limit) {
            trades.resize(limit);
        }
        return trades;
    }

    std::vector<models::Trade> TradeRepository::findBySymbolAndTimeRange(
        const std::string& symbol,
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        return mapper_.findByTimeRange(symbol, start, end);
    }

    double TradeRepository::getSymbolTotalVolume(
        const std::string& symbol,
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        return mapper_.getSymbolTotalVolume(symbol, start, end);
    }

    double TradeRepository::getSymbolAveragePrice(
        const std::string& symbol,
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        return mapper_.getSymbolAveragePrice(symbol, start, end);
    }

    void TradeRepository::saveBatch(const std::vector<models::Trade>& tradeList) {
        this->executeInTransaction([this, &tradeList](const TransactionPtr& transPtr) {
            for (const auto& trade : tradeList) {
                if (trade.getId() == 0) {
                    mapper_.insert(trade, *transPtr);
                } else {
                    mapper_.update(trade, *transPtr);
                }
            }
        });
    }

} // namespace repositories
