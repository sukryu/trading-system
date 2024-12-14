#include "models/factories/TradingSignalFactory.h"
#include <stdexcept>

namespace models {
    namespace factories {

        const TradingSignalFactory& TradingSignalFactory::getInstance() {
            static TradingSignalFactory instance;
            return instance;
        }

        std::shared_ptr<BaseModel> TradingSignalFactory::createFromDbRow(const drogon::orm::Row& row) const {
            try {
                auto signal = std::make_shared<TradingSignal>();
                *signal = TradingSignal::fromDbRow(row);
                return signal;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in TradingSignalFactory::createFromDbRow: ") + e.what());
            }
        }

        std::vector<std::shared_ptr<BaseModel>> TradingSignalFactory::createFromDbResult(const drogon::orm::Result& result) const {
            std::vector<std::shared_ptr<BaseModel>> signals;
            signals.reserve(result.size());
            
            for (const auto& row : result) {
                signals.push_back(createFromDbRow(row));
            }
            
            return signals;
        }

        std::shared_ptr<BaseModel> TradingSignalFactory::createFromJson(const Json::Value& json) const {
            try {
                auto signal = std::make_shared<TradingSignal>();
                signal->fromJson(json);
                return signal;
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in TradingSignalFactory::createFromJson: ") + e.what());
            }
        }

        std::shared_ptr<BaseModel> TradingSignalFactory::create() const {
            return std::make_shared<TradingSignal>();
        }

    } // namespace factories
} // namespace models