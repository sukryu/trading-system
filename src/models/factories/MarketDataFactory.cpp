#include "models/factories/MarketDataFactory.h"
#include <stdexcept>

namespace models {
    namespace factories {

        const MarketDataFactory& MarketDataFactory::getInstance() {
            static MarketDataFactory instance;
            return instance;
        }

        std::shared_ptr<BaseModel> MarketDataFactory::createFromDbRow(const drogon::orm::Row& row) const {
            auto marketData = std::make_shared<MarketData>();
            
            try {
                marketData->setId(row["id"].as<int64_t>());
                marketData->setSymbol(row["symbol"].as<std::string>());
                marketData->setPrice(row["price"].as<double>());
                marketData->setVolume(row["volume"].as<double>());
                marketData->setTimestamp(trantor::Date::fromDbString(row["timestamp"].as<std::string>()));
                marketData->setSource(row["source"].as<std::string>());
                marketData->setCreatedAt(trantor::Date::fromDbString(row["created_at"].as<std::string>()));
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error creating MarketData from DB row: ") + e.what());
            }
            
            return marketData;
        }

        std::vector<std::shared_ptr<BaseModel>> MarketDataFactory::createFromDbResult(const drogon::orm::Result& result) const {
            std::vector<std::shared_ptr<BaseModel>> marketDataList;
            marketDataList.reserve(result.size());
            
            for (const auto& row : result) {
                marketDataList.push_back(createFromDbRow(row));
            }
            
            return marketDataList;
        }

        std::shared_ptr<BaseModel> MarketDataFactory::createFromJson(const Json::Value& json) const {
            auto marketData = std::make_shared<MarketData>();
            
            try {
                marketData->fromJson(json);
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error creating MarketData from JSON: ") + e.what());
            }
            
            return marketData;
        }

        std::shared_ptr<BaseModel> MarketDataFactory::create() const {
            return std::make_shared<MarketData>();
        }

    }  // namespace factories
}  // namespace models