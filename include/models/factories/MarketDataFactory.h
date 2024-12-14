#pragma once

#include "models/ModelFactory.h"
#include "models/MarketData.h"

namespace models {
    namespace factories {

        class MarketDataFactory : public ModelFactory {
        public:
            std::shared_ptr<BaseModel> createFromDbRow(const drogon::orm::Row& row) const override;
            std::vector<std::shared_ptr<BaseModel>> createFromDbResult(const drogon::orm::Result& result) const override;
            std::shared_ptr<BaseModel> createFromJson(const Json::Value& json) const override;
            std::shared_ptr<BaseModel> create() const override;

            // Singleton 인스턴스 얻기
            static const MarketDataFactory& getInstance();

        private:
            MarketDataFactory() = default;
            ~MarketDataFactory() override = default;
            MarketDataFactory(const MarketDataFactory&) = delete;
            MarketDataFactory& operator=(const MarketDataFactory&) = delete;
        };

    }  // namespace factories
}  // namespace models