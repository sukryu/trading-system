#include "models/Order.h"
#include <stdexcept>

namespace models {

    Json::Value Order::toJson() const {
        Json::Value json;
        json["id"] = static_cast<Json::Int64>(id_);
        json["order_id"] = order_id_;
        json["symbol"] = symbol_;
        json["order_type"] = order_type_;
        json["side"] = side_;
        json["quantity"] = quantity_;
        json["price"] = price_;
        json["status"] = status_;
        json["signal_id"] = static_cast<Json::Int64>(signal_id_);
        json["filled_quantity"] = filled_quantity_;
        json["filled_price"] = filled_price_;
        json["error_message"] = error_message_;
        json["timestamp"] = timestamp_.toFormattedString(false);
        json["updated_at"] = updated_at_.toFormattedString(false);
        json["created_at"] = created_at_.toFormattedString(false);
        return json;
    }

    void Order::fromJson(const Json::Value& json) {
        if (json.isMember("id")) {
            id_ = json["id"].asInt64();
        }
        
        // 필수 필드 검증
        if (!json.isMember("symbol") || !json.isMember("order_type") ||
            !json.isMember("side") || !json.isMember("quantity")) {
            throw std::invalid_argument("Missing required fields in Order JSON");
        }

        order_id_ = json.get("order_id", "").asString();
        symbol_ = json["symbol"].asString();
        order_type_ = json["order_type"].asString();
        side_ = json["side"].asString();
        
        // 주문 타입 유효성 검사
        if (order_type_ != "MARKET" && order_type_ != "LIMIT") {
            throw std::invalid_argument("Invalid order_type. Must be 'MARKET' or 'LIMIT'");
        }

        // 주문 방향 유효성 검사
        if (side_ != "BUY" && side_ != "SELL") {
            throw std::invalid_argument("Invalid side. Must be 'BUY' or 'SELL'");
        }

        quantity_ = json["quantity"].asDouble();
        if (quantity_ <= 0) {
            throw std::invalid_argument("Quantity must be greater than 0");
        }

        // LIMIT 주문의 경우 가격이 필수
        if (order_type_ == "LIMIT") {
            if (!json.isMember("price")) {
                throw std::invalid_argument("Price is required for LIMIT orders");
            }
            price_ = json["price"].asDouble();
            if (price_ <= 0) {
                throw std::invalid_argument("Price must be greater than 0 for LIMIT orders");
            }
        }

        status_ = json.get("status", "PENDING").asString();
        
        // 주문 상태 유효성 검사
        const std::vector<std::string> valid_statuses = {"PENDING", "FILLED", "CANCELLED", "REJECTED"};
        if (std::find(valid_statuses.begin(), valid_statuses.end(), status_) == valid_statuses.end()) {
            throw std::invalid_argument("Invalid status value");
        }

        // 선택적 필드 처리
        if (json.isMember("signal_id")) {
            signal_id_ = json["signal_id"].asInt64();
        }

        filled_quantity_ = json.get("filled_quantity", 0.0).asDouble();
        filled_price_ = json.get("filled_price", 0.0).asDouble();
        error_message_ = json.get("error_message", "").asString();
        
        if (json.isMember("timestamp")) {
            timestamp_ = trantor::Date::fromDbString(json["timestamp"].asString());
        } else {
            timestamp_ = trantor::Date::now();
        }
        
        if (json.isMember("updated_at")) {
            updated_at_ = trantor::Date::fromDbString(json["updated_at"].asString());
        } else {
            updated_at_ = trantor::Date::now();
        }
        
        if (json.isMember("created_at")) {
            created_at_ = trantor::Date::fromDbString(json["created_at"].asString());
        } else {
            created_at_ = trantor::Date::now();
        }
    }

    Order Order::fromDbRow(const drogon::orm::Row& row) {
        Order order;
        
        try {
            order.setId(row["id"].as<int64_t>());
            order.setOrderId(row["order_id"].as<std::string>());
            order.setSymbol(row["symbol"].as<std::string>());
            order.setOrderType(row["order_type"].as<std::string>());
            order.setSide(row["side"].as<std::string>());
            order.setQuantity(row["quantity"].as<double>());
            order.setPrice(row["price"].as<double>());
            order.setStatus(row["status"].as<std::string>());
            order.setSignalId(row["signal_id"].as<int64_t>());
            order.setFilledQuantity(row["filled_quantity"].as<double>());
            order.setFilledPrice(row["filled_price"].as<double>());
            order.setErrorMessage(row["error_message"].as<std::string>());
            order.setTimestamp(trantor::Date::fromDbString(row["timestamp"].as<std::string>()));
            order.setUpdatedAt(trantor::Date::fromDbString(row["updated_at"].as<std::string>()));
            order.setCreatedAt(trantor::Date::fromDbString(row["created_at"].as<std::string>()));
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Error creating Order from DB row: ") + e.what());
        }
        
        return order;
    }

    std::vector<Order> Order::fromDbResult(const drogon::orm::Result& result) {
        std::vector<Order> orders;
        orders.reserve(result.size());
        
        for (const auto& row : result) {
            orders.push_back(fromDbRow(row));
        }
        
        return orders;
    }

} // namespace models