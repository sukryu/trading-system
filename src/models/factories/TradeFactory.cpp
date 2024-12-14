#include "models/factories/TradeFactory.h"
#include <stdexcept>

namespace models {
    namespace factories {

        const TradeFactory& TradeFactory::getInstance() {
            static TradeFactory instance;
            return instance;
        }

        std::shared_ptr<BaseModel> TradeFactory::createFromDbRow(const drogon::orm::Row& row) const {
            try {
                auto trade = std::make_shared<Trade>();
                *trade = Trade::fromDbRow(row);
                return trade;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in TradeFactory::createFromDbRow: ") + e.what());
            }
        }

        std::vector<std::shared_ptr<BaseModel>> TradeFactory::createFromDbResult(const drogon::orm::Result& result) const {
            std::vector<std::shared_ptr<BaseModel>> trades;
            trades.reserve(result.size());
            
            for (const auto& row : result) {
                trades.push_back(createFromDbRow(row));
            }
            
            return trades;
        }

        std::shared_ptr<BaseModel> TradeFactory::createFromJson(const Json::Value& json) const {
            try {
                auto trade = std::make_shared<Trade>();
                trade->fromJson(json);
                return trade;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in TradeFactory::createFromJson: ") + e.what());
            }
        }

        std::shared_ptr<BaseModel> TradeFactory::create() const {
            return std::make_shared<Trade>();
        }

    } // namespace factories
} // namespace models