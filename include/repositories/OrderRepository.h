#pragma once

#include "repositories/BaseRepository.h"
#include "models/Order.h"
#include "models/mappers/OrderMapper.h"
#include <string>
#include <optional>
#include <vector>

namespace repositories {

    class OrderRepository : public BaseRepository<models::Order> {
    public:
        using TransactionPtr = std::shared_ptr<drogon::orm::Transaction>;

        static OrderRepository& getInstance();

        // BaseRepository 구현
        models::Order save(const models::Order& order) override;
        std::optional<models::Order> findById(int64_t id) const override;
        bool deleteById(int64_t id) override;

        PaginationResult findAll(size_t page, size_t pageSize) const override;
        std::vector<models::Order> findByTimeRange(
            const trantor::Date& start,
            const trantor::Date& end
        ) const override;

        // Order 전용 메서드
        std::vector<models::Order> findBySymbol(const std::string& symbol, size_t limit = 100) const;
        std::vector<models::Order> findByStatus(const std::string& status, size_t limit = 100) const;
        std::vector<models::Order> findBySignalId(int64_t signalId) const;
        std::vector<models::Order> findPendingOrders(const std::string& symbol = "") const;

        // 특정 시간 범위 내 심볼별 조회 (OrderMapper 제공)
        std::vector<models::Order> findBySymbolAndTimeRange(
            const std::string& symbol,
            const trantor::Date& start,
            const trantor::Date& end
        ) const;

        // 주문 상태 업데이트
        void updateOrderStatus(int64_t id, const std::string& status, double filledQuantity = 0.0, double filledPrice = 0.0);

        // 벌크 작업 예시
        void saveBatch(const std::vector<models::Order>& orderList);

    private:
        OrderRepository() = default;
        ~OrderRepository() override = default;
        OrderRepository(const OrderRepository&) = delete;
        OrderRepository& operator=(const OrderRepository&) = delete;

        models::mappers::OrderMapper& mapper_{models::mappers::OrderMapper::getInstance()};
    };

} // namespace repositories
