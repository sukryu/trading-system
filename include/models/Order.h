#pragma once

#include "models/BaseModel.h"
#include <drogon/orm/Field.h>
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <trantor/utils/Date.h>
#include <string>

namespace models {

    class Order : public BaseModel {
    public:
        Order() = default;
        ~Order() override = default;

        // Getters
        int64_t getId() const { return id_; }
        const std::string& getOrderId() const { return order_id_; }
        const std::string& getSymbol() const { return symbol_; }
        const std::string& getOrderType() const { return order_type_; }
        const std::string& getSide() const { return side_; }
        double getQuantity() const { return quantity_; }
        double getPrice() const { return price_; }
        const std::string& getStatus() const { return status_; }
        int64_t getSignalId() const { return signal_id_; }
        double getFilledQuantity() const { return filled_quantity_; }
        double getFilledPrice() const { return filled_price_; }
        const std::string& getErrorMessage() const { return error_message_; }
        const trantor::Date& getTimestamp() const { return timestamp_; }
        const trantor::Date& getUpdatedAt() const { return updated_at_; }
        const trantor::Date& getCreatedAt() const { return created_at_; }

        // Setters
        void setId(int64_t id) { id_ = id; }
        void setOrderId(const std::string& order_id) { order_id_ = order_id; }
        void setSymbol(const std::string& symbol) { symbol_ = symbol; }
        void setOrderType(const std::string& order_type) { order_type_ = order_type; }
        void setSide(const std::string& side) { side_ = side; }
        void setQuantity(double quantity) { quantity_ = quantity; }
        void setPrice(double price) { price_ = price; }
        void setStatus(const std::string& status) { status_ = status; }
        void setSignalId(int64_t signal_id) { signal_id_ = signal_id; }
        void setFilledQuantity(double filled_quantity) { filled_quantity_ = filled_quantity; }
        void setFilledPrice(double filled_price) { filled_price_ = filled_price; }
        void setErrorMessage(const std::string& error_message) { error_message_ = error_message; }
        void setTimestamp(const trantor::Date& timestamp) { timestamp_ = timestamp; }
        void setUpdatedAt(const trantor::Date& updated_at) { updated_at_ = updated_at; }
        void setCreatedAt(const trantor::Date& created_at) { created_at_ = created_at; }

        // JSON conversion
        Json::Value toJson() const override;
        void fromJson(const Json::Value& json) override;

        // Static factory methods
        static Order fromDbRow(const drogon::orm::Row& row);
        static std::vector<Order> fromDbResult(const drogon::orm::Result& result);

    private:
        int64_t id_{0};
        std::string order_id_;
        std::string symbol_;
        std::string order_type_;  // MARKET, LIMIT
        std::string side_;        // BUY, SELL
        double quantity_{0.0};
        double price_{0.0};
        std::string status_;      // PENDING, FILLED, CANCELLED, REJECTED
        int64_t signal_id_{0};
        double filled_quantity_{0.0};
        double filled_price_{0.0};
        std::string error_message_;
        trantor::Date timestamp_;
        trantor::Date updated_at_;
        trantor::Date created_at_;
    };

}