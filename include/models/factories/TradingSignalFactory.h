#pragma once

#include "models/ModelFactory.h"
#include "models/TradingSignal.h"

namespace models {
    namespace factories {

        class TradingSignalFactory : public ModelFactory {
        public:
            std::shared_ptr<BaseModel> createFromDbRow(const drogon::orm::Row& row) const override;
            std::vector<std::shared_ptr<BaseModel>> createFromDbResult(const drogon::orm::Result& result) const override;
            std::shared_ptr<BaseModel> createFromJson(const Json::Value& json) const override;
            std::shared_ptr<BaseModel> create() const override;

            // Singleton 인스턴스 얻기
            static const TradingSignalFactory& getInstance();

        private:
            TradingSignalFactory() = default;
            ~TradingSignalFactory() override = default;
            TradingSignalFactory(const TradingSignalFactory&) = delete;
            TradingSignalFactory& operator=(const TradingSignalFactory&) = delete;
        };

    } // namespace factories
} // namespace models