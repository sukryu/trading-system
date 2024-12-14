#include "models/MarketData.h"
#include <stdexcept>

namespace models {

    Json::Value MarketData::toJson() const {
        Json::Value json;
        json["id"] = static_cast<Json::Int64>(id_);
        json["symbol"] = symbol_;
        json["price"] = price_;
        json["volume"] = volume_;
        json["timestamp"] = timestamp_.toFormattedString(false);
        json["source"] = source_;
        json["created_at"] = created_at_.toFormattedString(false);
        return json;
    }

    void MarketData::fromJson(const Json::Value& json) {
        if (json.isMember("id")) {
            id_ = json["id"].asInt64();
        }
        
        if (!json.isMember("symbol") || !json.isMember("price") || !json.isMember("volume")) {
            throw std::invalid_argument("Missing required fields in JSON");
        }

        symbol_ = json["symbol"].asString();
        price_ = json["price"].asDouble();
        volume_ = json["volume"].asDouble();
        
        if (json.isMember("timestamp")) {
            timestamp_ = trantor::Date::fromDbString(json["timestamp"].asString());
        }
        
        if (json.isMember("source")) {
            source_ = json["source"].asString();
        }
        
        if (json.isMember("created_at")) {
            created_at_ = trantor::Date::fromDbString(json["created_at"].asString());
        }
    }

    MarketData MarketData::fromDbRow(const drogon::orm::Row& row) {
        MarketData data;
        data.setId(row["id"].as<int64_t>());
        data.setSymbol(row["symbol"].as<std::string>());
        data.setPrice(row["price"].as<double>());
        data.setVolume(row["volume"].as<double>());
        data.setTimestamp(trantor::Date::fromDbString(row["timestamp"].as<std::string>()));
        data.setSource(row["source"].as<std::string>());
        data.setCreatedAt(trantor::Date::fromDbString(row["created_at"].as<std::string>()));
        return data;
    }

    std::vector<MarketData> MarketData::fromDbResult(const drogon::orm::Result& result) {
        std::vector<MarketData> marketDataList;
        marketDataList.reserve(result.size());
        
        for (const auto& row : result) {
            marketDataList.push_back(fromDbRow(row));
        }
        
        return marketDataList;
    }

}