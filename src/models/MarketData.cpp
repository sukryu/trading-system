#include "models/MarketData.h"
#include "utils/Logger.h"
#include <stdexcept>

namespace models {

    MarketData::MarketData(const MarketData& other) {
        id_.store(other.id_.load(std::memory_order_relaxed));
        strncpy(symbol_, other.symbol_, sizeof(symbol_));
        symbol_[sizeof(symbol_) - 1] = '\0';
        price_.store(other.price_.load(std::memory_order_relaxed));
        volume_.store(other.volume_.load(std::memory_order_relaxed));
        timestamp_ = other.timestamp_;
        strncpy(source_, other.source_, sizeof(source_));
        source_[sizeof(source_) - 1] = '\0';
        created_at_ = other.created_at_;
    }


    memory::MemoryPool<MarketData>& MarketData::memory_pool() {
        static memory::MemoryPool<MarketData> pool(1024);
        return pool;
    }

    containers::LockFreeQueue<std::shared_ptr<MarketData>>& MarketData::update_queue() {
        static containers::LockFreeQueue<std::shared_ptr<MarketData>> queue;
        return queue;
    }

    std::shared_ptr<MarketData> MarketData::create(
        const std::string& symbol,
        double price,
        double volume,
        const std::string& source
    ) {
        auto ptr = MarketData::memory_pool().allocate();
        ptr->setSymbol(symbol);
        ptr->setPrice(price);
        ptr->setVolume(volume);
        ptr->setSource(source);
        ptr->setTimestamp(trantor::Date::now());
        return std::shared_ptr<MarketData>(
            ptr,
            [](MarketData* p) { MarketData::memory_pool().deallocate(p); }
        );
    }

    Json::Value MarketData::toJson() const {
        Json::Value json;
        json["id"] = static_cast<Json::Int64>(id_.load(std::memory_order_relaxed));
        json["symbol"] = symbol_;
        json["price"] = price_.load(std::memory_order_relaxed);
        json["volume"] = volume_.load(std::memory_order_relaxed);
        json["timestamp"] = timestamp_.toFormattedString(false);
        json["source"] = source_;
        json["created_at"] = created_at_.toFormattedString(false);
        return json;
    }

    void MarketData::fromJson(const Json::Value& json) {
        try {
            if (json.isMember("id")) {
                setId(json["id"].asInt64());
            }

            // 필수 필드 검증
            if (!json.isMember("symbol") || !json.isMember("price") || 
                !json.isMember("volume")) {
                throw std::invalid_argument("Missing required fields in MarketData JSON");
            }

            setSymbol(json["symbol"].asString());
            setPrice(json["price"].asDouble());
            setVolume(json["volume"].asDouble());

            if (json.isMember("timestamp")) {
                setTimestamp(trantor::Date::fromDbString(json["timestamp"].asString()));
            } else {
                setTimestamp(trantor::Date::now());
            }

            if (json.isMember("source")) {
                setSource(json["source"].asString());
            }

            if (json.isMember("created_at")) {
                setCreatedAt(trantor::Date::fromDbString(json["created_at"].asString()));
            } else {
                setCreatedAt(trantor::Date::now());
            }

        } catch (const std::exception& e) {
            TRADING_LOG_ERROR("Error parsing MarketData from JSON: {}", e.what());
            throw;
        }
    }

    std::shared_ptr<MarketData> MarketData::fromDbRow(const drogon::orm::Row& row) {
        auto data = MarketData::memory_pool().allocate();
        try {
            data->setId(row["id"].as<int64_t>());
            data->setSymbol(row["symbol"].as<std::string>());
            data->setPrice(row["price"].as<double>());
            data->setVolume(row["volume"].as<double>());
            data->setTimestamp(trantor::Date::fromDbString(row["timestamp"].as<std::string>()));
            data->setSource(row["source"].as<std::string>());
            data->setCreatedAt(trantor::Date::fromDbString(row["created_at"].as<std::string>()));
        } catch (const std::exception& e) {
            TRADING_LOG_ERROR("Error creating MarketData from DB row: {}", e.what());
            MarketData::memory_pool().deallocate(data); // 메모리 풀 반환
            throw;
        }
        return std::shared_ptr<MarketData>(
            data, 
            [](MarketData* p) { MarketData::memory_pool().deallocate(p); }
        );
    }

    std::vector<std::shared_ptr<MarketData>> MarketData::fromDbResult(
    const drogon::orm::Result& result) {
        std::vector<std::shared_ptr<MarketData>> marketDataList;
        marketDataList.reserve(result.size());

        for (const auto& row : result) {
            try {
                marketDataList.push_back(fromDbRow(row));
            } catch (const std::exception& e) {
                TRADING_LOG_ERROR("Error processing row in batch: {}", e.what());
                // 개별 실패는 기록하고 계속 진행
                continue;
            }
        }

        return marketDataList;
    }
} // namespace models