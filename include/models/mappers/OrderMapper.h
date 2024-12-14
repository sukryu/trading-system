#pragma once

#include <drogon/drogon.h>
#include "models/mappers/BaseMapper.h"
#include "models/Order.h"

namespace models {
    namespace mappers {

        class OrderMapper : public BaseMapper<Order> {
        public:
            using Transaction = drogon::orm::Transaction;

            static OrderMapper& getInstance();

            // 기본 CRUD 오버로드 (트랜잭션 지원)
            Order insert(const Order& order) override;
            Order insert(const Order& order, Transaction& trans);

            Order findById(int64_t id) override;
            std::vector<Order> findAll() override;
            std::vector<Order> findByCriteria(const std::string& whereClause) override;
            std::vector<Order> findByCriteria(const std::string& whereClause, Transaction& trans);
            
            void update(const Order& order) override;
            void update(const Order& order, Transaction& trans);
            
            void deleteById(int64_t id) override;
            void deleteById(int64_t id, Transaction& trans);
            
            size_t count(const std::string& whereClause = "") override;
            std::vector<Order> findWithPaging(size_t limit, size_t offset) override;

            // Order 전용 메서드
            std::vector<Order> findBySymbol(const std::string& symbol);
            std::vector<Order> findBySymbol(const std::string& symbol, Transaction& trans);

            std::vector<Order> findByStatus(const std::string& status);
            std::vector<Order> findByStatus(const std::string& status, Transaction& trans);

            std::vector<Order> findBySignalId(int64_t signalId);
            std::vector<Order> findBySignalId(int64_t signalId, Transaction& trans);

            std::vector<Order> findByTimeRange(
                const std::string& symbol,
                const trantor::Date& start,
                const trantor::Date& end
            );
            std::vector<Order> findByTimeRange(
                const std::string& symbol,
                const trantor::Date& start,
                const trantor::Date& end,
                Transaction& trans
            );

            std::vector<Order> findPendingOrders(const std::string& symbol = "");
            std::vector<Order> findPendingOrders(const std::string& symbol, Transaction& trans);

            void updateOrderStatus(int64_t id, const std::string& status, 
                                   double filledQuantity = 0.0, double filledPrice = 0.0);
            void updateOrderStatus(int64_t id, const std::string& status, 
                                   double filledQuantity, double filledPrice,
                                   Transaction& trans);

        private:
            OrderMapper() = default;
            ~OrderMapper() override = default;
            OrderMapper(const OrderMapper&) = delete;
            OrderMapper& operator=(const OrderMapper&) = delete;

            drogon::orm::DbClientPtr getDbClient() const {
                return drogon::app().getDbClient();
            }
        };

    } // namespace mappers
} // namespace models
