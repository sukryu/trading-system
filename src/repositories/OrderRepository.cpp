#include "repositories/OrderRepository.h"
#include <stdexcept>
#include <sstream>
#include <cmath>

namespace repositories {

    OrderRepository& OrderRepository::getInstance() {
        static OrderRepository instance;
        return instance;
    }

    models::Order OrderRepository::save(const models::Order& order) {
        // id가 0이면 새 레코드 삽입, 아니면 업데이트
        if (order.getId() == 0) {
            return mapper_.insert(order);
        } else {
            mapper_.update(order);
            return order;
        }
    }

    std::optional<models::Order> OrderRepository::findById(int64_t id) const {
        try {
            return std::make_optional(mapper_.findById(id));
        } catch (const std::runtime_error&) {
            return std::nullopt;
        }
    }

    bool OrderRepository::deleteById(int64_t id) {
        try {
            mapper_.deleteById(id);
            return true;
        } catch (const std::runtime_error&) {
            return false;
        }
    }

    BaseRepository<models::Order>::PaginationResult 
    OrderRepository::findAll(size_t page, size_t pageSize) const {
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

    std::vector<models::Order> OrderRepository::findByTimeRange(
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        // 현재 OrderMapper는 심볼을 지정해야 시간 범위 조회가 가능하다고 가정.
        // 심볼 없이 전체 오더의 시간 범위 조회가 필요하다면 OrderMapper를 수정하거나 여기서 예외 처리.
        throw std::runtime_error("findByTimeRange without symbol is not supported by the current mapper");
    }

    // Order 전용 메서드
    std::vector<models::Order> OrderRepository::findBySymbol(const std::string& symbol, size_t limit) const {
        // mapper에 limit 지원이 없다면 findBySymbol 후 상위 limit만 slicing
        // 여기서는 mapper에 limit 기능이 없으므로 전체를 받고 상위 limit만 잘라냄
        auto result = mapper_.findBySymbol(symbol);
        if (result.size() > limit) {
            result.resize(limit);
        }
        return result;
    }

    std::vector<models::Order> OrderRepository::findByStatus(const std::string& status, size_t limit) const {
        auto result = mapper_.findByStatus(status);
        if (result.size() > limit) {
            result.resize(limit);
        }
        return result;
    }

    std::vector<models::Order> OrderRepository::findBySignalId(int64_t signalId) const {
        return mapper_.findBySignalId(signalId);
    }

    std::vector<models::Order> OrderRepository::findPendingOrders(const std::string& symbol) const {
        return mapper_.findPendingOrders(symbol);
    }

    std::vector<models::Order> OrderRepository::findBySymbolAndTimeRange(
        const std::string& symbol,
        const trantor::Date& start,
        const trantor::Date& end
    ) const {
        return mapper_.findByTimeRange(symbol, start, end);
    }

    void OrderRepository::updateOrderStatus(int64_t id, const std::string& status, double filledQuantity, double filledPrice) {
        mapper_.updateOrderStatus(id, status, filledQuantity, filledPrice);
    }

    // Batch 작업 예: 한 번에 여러 Order를 Insert/Update
    void OrderRepository::saveBatch(const std::vector<models::Order>& orderList) {
        this->executeInTransaction([this, &orderList](const TransactionPtr& transPtr) {
            for (const auto& order : orderList) {
                if (order.getId() == 0) {
                    mapper_.insert(order, *transPtr);
                } else {
                    mapper_.update(order, *transPtr);
                }
            }
        });
    }

} // namespace repositories
