#include "models/TradingSignal.h"
#include <stdexcept>

namespace models {

    Json::Value TradingSignal::toJson() const {
        Json::Value json;
        json["id"] = static_cast<Json::Int64>(id_);
        json["symbol"] = symbol_;
        json["signal_type"] = signal_type_;
        json["price"] = price_;
        json["quantity"] = quantity_;
        json["strategy_name"] = strategy_name_;
        json["confidence"] = confidence_;
        json["parameters"] = parameters_;
        json["timestamp"] = timestamp_.toFormattedString(false);
        json["created_at"] = created_at_.toFormattedString(false);
        return json;
    }

    void TradingSignal::fromJson(const Json::Value& json) {
        if (json.isMember("id")) {
            id_ = json["id"].asInt64();
        }
        
        // 필수 필드 검증
        if (!json.isMember("symbol") || !json.isMember("signal_type") ||
            !json.isMember("price") || !json.isMember("quantity") ||
            !json.isMember("strategy_name")) {
            throw std::invalid_argument("Missing required fields in TradingSignal JSON");
        }

        // 필드 값 설정
        symbol_ = json["symbol"].asString();
        signal_type_ = json["signal_type"].asString();
        
        // signal_type 유효성 검사
        if (signal_type_ != "BUY" && signal_type_ != "SELL") {
            throw std::invalid_argument("Invalid signal_type. Must be 'BUY' or 'SELL'");
        }

        price_ = json["price"].asDouble();
        quantity_ = json["quantity"].asDouble();
        strategy_name_ = json["strategy_name"].asString();
        
        // 선택적 필드 처리
        if (json.isMember("confidence")) {
            confidence_ = json["confidence"].asDouble();
            // confidence 범위 검증 (0.0 ~ 100.0)
            if (confidence_ < 0.0 || confidence_ > 100.0) {
                throw std::invalid_argument("Confidence must be between 0 and 100");
            }
        }

        if (json.isMember("parameters")) {
            parameters_ = json["parameters"];
        }
        
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

    TradingSignal TradingSignal::fromDbRow(const drogon::orm::Row& row) {
        TradingSignal signal;
        
        try {
            signal.setId(row["id"].as<int64_t>());
            signal.setSymbol(row["symbol"].as<std::string>());
            signal.setSignalType(row["signal_type"].as<std::string>());
            signal.setPrice(row["price"].as<double>());
            signal.setQuantity(row["quantity"].as<double>());
            signal.setStrategyName(row["strategy_name"].as<std::string>());
            signal.setConfidence(row["confidence"].as<double>());
            
            // JSONB 필드 파싱
            Json::Reader reader;
            Json::Value parameters;
            if (reader.parse(row["parameters"].as<std::string>(), parameters)) {
                signal.setParameters(parameters);
            }
            
            signal.setTimestamp(trantor::Date::fromDbString(row["timestamp"].as<std::string>()));
            signal.setCreatedAt(trantor::Date::fromDbString(row["created_at"].as<std::string>()));
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Error creating TradingSignal from DB row: ") + e.what());
        }
        
        return signal;
    }

    std::vector<TradingSignal> TradingSignal::fromDbResult(const drogon::orm::Result& result) {
        std::vector<TradingSignal> signals;
        signals.reserve(result.size());
        
        for (const auto& row : result) {
            signals.push_back(fromDbRow(row));
        }
        
        return signals;
    }

} // namespace models