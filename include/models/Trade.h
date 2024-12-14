#pragma once

#include "models/BaseModel.h"
#include <drogon/orm/Field.h>
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <trantor/utils/Date.h>
#include <string>

namespace models {

    class Trade : public BaseModel {
    public:
        Trade() = default;
        ~Trade() override = default;

        // Getters
        int64_t getId() const { return id_; }
        const std::string& getTradeId() const { return trade_id_; }
        int64_t getOrderId() const { return order_id_; }
        const std::string& getSymbol() const { return symbol_; }
        const std::string& getSide() const { return side_; }
        double getQuantity() const { return quantity_; }
        double getPrice() const { return price_; }
        double getCommission() const { return commission_; }
        const std::string& getCommissionAsset() const { return commission_asset_; }
        const trantor::Date& getTimestamp() const { return timestamp_; }
        const trantor::Date& getCreatedAt() const { return created_at_; }

        // Setters
        void setId(int64_t id) { id_ = id; }
        void setTradeId(const std::string& trade_id) { trade_id_ = trade_id; }
        void setOrderId(int64_t order_id) { order_id_ = order_id; }
        void setSymbol(const std::string& symbol) { symbol_ = symbol; }
        void setSide(const std::string& side) { side_ = side; }
        void setQuantity(double quantity) { quantity_ = quantity; }
        void setPrice(double price) { price_ = price; }
        void setCommission(double commission) { commission_ = commission; }
        void setCommissionAsset(const std::string& commission_asset) { commission_asset_ = commission_asset; }
        void setTimestamp(const trantor::Date& timestamp) { timestamp_ = timestamp; }
        void setCreatedAt(const trantor::Date& created_at) { created_at_ = created_at; }

        // JSON conversion
        Json::Value toJson() const override;
        void fromJson(const Json::Value& json) override;

        // Static factory methods
        static Trade fromDbRow(const drogon::orm::Row& row);
        static std::vector<Trade> fromDbResult(const drogon::orm::Result& result);

    private:
        int64_t id_{0};
        std::string trade_id_;
        int64_t order_id_{0};
        std::string symbol_;
        std::string side_;  // BUY, SELL
        double quantity_{0.0};
        double price_{0.0};
        double commission_{0.0};
        std::string commission_asset_;
        trantor::Date timestamp_;
        trantor::Date created_at_;
    };

}