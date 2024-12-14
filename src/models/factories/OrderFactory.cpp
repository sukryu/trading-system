#include "models/factories/OrderFactory.h"
#include <stdexcept>

namespace models {
    namespace factories {

        const OrderFactory& OrderFactory::getInstance() {
            static OrderFactory instance;
            return instance;
        }

        std::shared_ptr<BaseModel> OrderFactory::createFromDbRow(const drogon::orm::Row& row) const {
            try {
                auto order = std::make_shared<Order>();
                *order = Order::fromDbRow(row);
                return order;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in OrderFactory::createFromDbRow: ") + e.what());
            }
        }

        std::vector<std::shared_ptr<BaseModel>> OrderFactory::createFromDbResult(const drogon::orm::Result& result) const {
            std::vector<std::shared_ptr<BaseModel>> orders;
            orders.reserve(result.size());
            
            for (const auto& row : result) {
                orders.push_back(createFromDbRow(row));
            }
            
            return orders;
        }

        std::shared_ptr<BaseModel> OrderFactory::createFromJson(const Json::Value& json) const {
            try {
                auto order = std::make_shared<Order>();
                order->fromJson(json);
                return order;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in OrderFactory::createFromJson: ") + e.what());
            }
        }

        std::shared_ptr<BaseModel> OrderFactory::create() const {
            return std::make_shared<Order>();
        }

    } // namespace factories
} // namespace models