#pragma once

#include "models/BaseModel.h"
#include <drogon/orm/Field.h>
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <trantor/utils/Date.h>
#include <string>

namespace models {

    class TradingSignal : public BaseModel {
    public:
        TradingSignal() = default;
        ~TradingSignal() override = default;

        // Getters
        int64_t getId() const { return id_; }
        const std::string& getSymbol() const { return symbol_; }
        const std::string& getSignalType() const { return signal_type_; }
        double getPrice() const { return price_; }
        double getQuantity() const { return quantity_; }
        const std::string& getStrategyName() const { return strategy_name_; }
        double getConfidence() const { return confidence_; }
        const Json::Value& getParameters() const { return parameters_; }
        const trantor::Date& getTimestamp() const { return timestamp_; }
        const trantor::Date& getCreatedAt() const { return created_at_; }

        // Setters
        void setId(int64_t id) { id_ = id; }
        void setSymbol(const std::string& symbol) { symbol_ = symbol; }
        void setSignalType(const std::string& signal_type) { signal_type_ = signal_type; }
        void setPrice(double price) { price_ = price; }
        void setQuantity(double quantity) { quantity_ = quantity; }
        void setStrategyName(const std::string& strategy_name) { strategy_name_ = strategy_name; }
        void setConfidence(double confidence) { confidence_ = confidence; }
        void setParameters(const Json::Value& parameters) { parameters_ = parameters; }
        void setTimestamp(const trantor::Date& timestamp) { timestamp_ = timestamp; }
        void setCreatedAt(const trantor::Date& created_at) { created_at_ = created_at; }

        // JSON conversion
        Json::Value toJson() const override;
        void fromJson(const Json::Value& json) override;

        // Static factory methods
        static TradingSignal fromDbRow(const drogon::orm::Row& row);
        static std::vector<TradingSignal> fromDbResult(const drogon::orm::Result& result);

    private:
        int64_t id_{0};
        std::string symbol_;
        std::string signal_type_;  // BUY, SELL
        double price_{0.0};
        double quantity_{0.0};
        std::string strategy_name_;  // MA, RSI, etc.
        double confidence_{0.0};
        Json::Value parameters_;
        trantor::Date timestamp_;
        trantor::Date created_at_;
    };

}