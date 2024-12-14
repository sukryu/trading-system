#include "repositories/TradingSignalRepository.h"
#include <stdexcept>
#include <cmath>

namespace repositories {

    TradingSignalRepository& TradingSignalRepository::getInstance() {
        static TradingSignalRepository instance;
        return instance;
    }

    models::TradingSignal TradingSignalRepository::save(const models::TradingSignal& signal) {
        if (signal.getId() == 0) {
            return mapper_.insert(signal);
        } else {
            mapper_.update(signal);
            return signal;
        }
    }

    std::optional<models::TradingSignal> TradingSignalRepository::findById(int64_t id) const {
        try {
            return std::make_optional(mapper_.findById(id));
        } catch (const std::runtime_error&) {
            return std::nullopt;
        }
    }

    bool TradingSignalRepository::deleteById(int64_t id) {
        try {
            mapper_.deleteById(id);
            return true;
        } catch (const std::runtime_error&) {
            return false;
        }
    }

    BaseRepository<models::TradingSignal>::PaginationResult
    TradingSignalRepository::findAll(size_t page, size_t pageSize) const {
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

    std::vector<models::TradingSignal> TradingSignalRepository::findByTimeRange(
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        // TradingSignalMapper는 symbol이 필요한 time range 조회 메서드가 있음.
        // 전체 심볼 대상 time range 조회가 없다면 예외 처리
        throw std::runtime_error("findByTimeRange without symbol is not supported by the current mapper");
    }

    std::vector<models::TradingSignal> TradingSignalRepository::findBySymbol(
        const std::string& symbol,
        size_t limit
    ) const {
        // findBySymbol 메서드 호출 후 limit 적용
        auto signals = mapper_.findBySymbol(symbol);
        if (signals.size() > limit) {
            signals.resize(limit);
        }
        return signals;
    }

    std::vector<models::TradingSignal> TradingSignalRepository::findByStrategyName(
        const std::string& strategyName,
        size_t limit
    ) const {
        auto signals = mapper_.findByStrategyName(strategyName);
        if (signals.size() > limit) {
            signals.resize(limit);
        }
        return signals;
    }

    std::vector<models::TradingSignal> TradingSignalRepository::findPendingSignals(const std::string& symbol) const {
        return mapper_.findPendingSignals(symbol);
    }

    std::vector<models::TradingSignal> TradingSignalRepository::findBySymbolAndTimeRange(
        const std::string& symbol,
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        return mapper_.findByTimeRange(symbol, start, end);
    }

    void TradingSignalRepository::saveBatch(const std::vector<models::TradingSignal>& signalList) {
        this->executeInTransaction([this, &signalList](const TransactionPtr& transPtr) {
            for (const auto& signal : signalList) {
                if (signal.getId() == 0) {
                    mapper_.insert(signal, *transPtr);
                } else {
                    mapper_.update(signal, *transPtr);
                }
            }
        });
    }

} // namespace repositories
