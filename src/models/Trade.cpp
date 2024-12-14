#include "models/Trade.h"
#include <stdexcept>

namespace models {

    Json::Value Trade::toJson() const {
        Json::Value json;
        json["id"] = static_cast<Json::Int64>(id_);
        json["trade_id"] = trade_id_;
        json["order_id"] = static_cast<Json::Int64>(order_id_);
        json["symbol"] = symbol_;
        json["side"] = side_;
        json["quantity"] = quantity_;
        json["price"] = price_;
        json["commission"] = commission_;
        json["commission_asset"] = commission_asset_;
        json["timestamp"] = timestamp_.toFormattedString(false);
        json["created_at"] = created_at_.toFormattedString(false);
        return json;
    }

    void Trade::fromJson(const Json::Value& json) {
        if (json.isMember("id")) {
            id_ = json["id"].asInt64();
        }
        
        // 필수 필드 검증
        if (!json.isMember("symbol") || !json.isMember("side") ||
            !json.isMember("quantity") || !json.isMember("price")) {
            throw std::invalid_argument("Missing required fields in Trade JSON");
        }

        if (json.isMember("trade_id")) {
            trade_id_ = json["trade_id"].asString();
        }

        if (json.isMember("order_id")) {
            order_id_ = json["order_id"].asInt64();
        }

        symbol_ = json["symbol"].asString();
        side_ = json["side"].asString();
        
        // 거래 방향 유효성 검사
        if (side_ != "BUY" && side_ != "SELL") {
            throw std::invalid_argument("Invalid side. Must be 'BUY' or 'SELL'");
        }

        // 수량 유효성 검사
        quantity_ = json["quantity"].asDouble();
        if (quantity_ <= 0) {
            throw std::invalid_argument("Quantity must be greater than 0");
        }

        // 가격 유효성 검사
        price_ = json["price"].asDouble();
        if (price_ <= 0) {
            throw std::invalid_argument("Price must be greater than 0");
        }

        // 수수료 정보 처리 (선택적)
        if (json.isMember("commission")) {
            commission_ = json["commission"].asDouble();
            if (commission_ < 0) {
                throw std::invalid_argument("Commission cannot be negative");
            }
            
            // 수수료가 있는 경우 수수료 자산 정보는 필수
            if (!json.isMember("commission_asset")) {
                throw std::invalid_argument("Commission asset is required when commission is provided");
            }
            commission_asset_ = json["commission_asset"].asString();
        }
        
        // 시간 정보 처리
        if (json.isMember("timestamp")) {
            timestamp_ = trantor::Date::fromDbString(json["timestamp"].asString());
        } else {
            timestamp_ = trantor::Date::now();
        }
        
        if (json.isMember("created_at")) {
            created_at_ = trantor::Date::fromDbString(json["created_at"].asString());
        } else {
            created_at_ = trantor::Date::now();
        }
    }

    Trade Trade::fromDbRow(const drogon::orm::Row& row) {
        Trade trade;
        
        try {
            trade.setId(row["id"].as<int64_t>());
            trade.setTradeId(row["trade_id"].as<std::string>());
            trade.setOrderId(row["order_id"].as<int64_t>());
            trade.setSymbol(row["symbol"].as<std::string>());
            trade.setSide(row["side"].as<std::string>());
            trade.setQuantity(row["quantity"].as<double>());
            trade.setPrice(row["price"].as<double>());
            trade.setCommission(row["commission"].as<double>());
            trade.setCommissionAsset(row["commission_asset"].as<std::string>());
            trade.setTimestamp(trantor::Date::fromDbString(row["timestamp"].as<std::string>()));
            trade.setCreatedAt(trantor::Date::fromDbString(row["created_at"].as<std::string>()));
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Error creating Trade from DB row: ") + e.what());
        }
        
        return trade;
    }

    std::vector<Trade> Trade::fromDbResult(const drogon::orm::Result& result) {
        std::vector<Trade> trades;
        trades.reserve(result.size());
        
        for (const auto& row : result) {
            trades.push_back(fromDbRow(row));
        }
        
        return trades;
    }

} // namespace models