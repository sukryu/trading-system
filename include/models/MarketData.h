#pragma once

#include "models/BaseModel.h"
#include <drogon/orm/Field.h>
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <trantor/utils/Date.h>
#include <string>

namespace models {

    class MarketData : public BaseModel {
    public:
        MarketData() = default;
        ~MarketData() override = default;

        // Getters
        int64_t getId() const { return id_; }
        const std::string& getSymbol() const { return symbol_; }
        double getPrice() const { return price_; }
        double getVolume() const { return volume_; }
        const trantor::Date& getTimestamp() const { return timestamp_; }
        const std::string& getSource() const { return source_; }
        const trantor::Date& getCreatedAt() const { return created_at_; }

        // Setters
        void setId(int64_t id) { id_ = id; }
        void setSymbol(const std::string& symbol) { symbol_ = symbol; }
        void setPrice(double price) { price_ = price; }
        void setVolume(double volume) { volume_ = volume; }
        void setTimestamp(const trantor::Date& timestamp) { timestamp_ = timestamp; }
        void setSource(const std::string& source) { source_ = source; }
        void setCreatedAt(const trantor::Date& created_at) { created_at_ = created_at; }

        // JSON conversion
        Json::Value toJson() const override;
        void fromJson(const Json::Value& json) override;

        // Static factory methods
        static MarketData fromDbRow(const drogon::orm::Row& row);
        static std::vector<MarketData> fromDbResult(const drogon::orm::Result& result);

    private:
        int64_t id_{0};
        std::string symbol_;
        double price_{0.0};
        double volume_{0.0};
        trantor::Date timestamp_;
        std::string source_;
        trantor::Date created_at_;
    };

}