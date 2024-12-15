#pragma once

#include "models/BaseModel.h"
#include <drogon/drogon.h>
#include <drogon/orm/Field.h>
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <trantor/utils/Date.h>
#include <string>
#include "containers/LockFreeContainers.h"
#include <atomic>
#include "memory/MemoryPool.h"

namespace models {

    class MarketData : public BaseModel {
    public:
        MarketData() = default;
        MarketData(const MarketData& other); // 복사 생성자 추가

        // 메모리 풀에서 객체 생성을 위한 팩토리 메서드
        static std::shared_ptr<MarketData> create(
            const std::string& symbol,
            double price,
            double volume,
            const std::string& source
        );

        // Getters
        int64_t getId() const { return id_.load(std::memory_order_relaxed); }
        std::string_view getSymbol() const { return symbol_; }
        double getPrice() const { return price_.load(std::memory_order_relaxed); }
        double getVolume() const { return volume_.load(std::memory_order_relaxed); }
        const trantor::Date& getTimestamp() const { return timestamp_; }
        std::string_view getSource() const { return source_; }
        const trantor::Date& getCreatedAt() const { return created_at_; }

        // Setters with atomic operations
        void setId(int64_t id) { id_.store(id, std::memory_order_relaxed); }
        void setSymbol(const std::string& symbol) { 
            strncpy(symbol_, symbol.c_str(), sizeof(symbol_) - 1);
            symbol_[sizeof(symbol_) - 1] = '\0';
        }
        void setPrice(double price) { price_.store(price, std::memory_order_relaxed); }
        void setVolume(double volume) { volume_.store(volume, std::memory_order_relaxed); }
        void setTimestamp(const trantor::Date& timestamp) { timestamp_ = timestamp; }
        void setSource(const std::string& source) {
            strncpy(source_, source.c_str(), sizeof(source_) - 1);
            source_[sizeof(source_) - 1] = '\0';
        }
        void setCreatedAt(const trantor::Date& created_at) { created_at_ = created_at; }

        // JSON conversion
        Json::Value toJson() const override;
        void fromJson(const Json::Value& json) override;

        // Static factory methods for database operations
        static std::shared_ptr<MarketData> fromDbRow(const drogon::orm::Row& row);
        static std::vector<std::shared_ptr<MarketData>> fromDbResult(const drogon::orm::Result& result);

        // 증분 업데이트를 위한 메서드
        bool updatePriceIfChanged(double newPrice) {
            double oldPrice = price_.load(std::memory_order_relaxed);
            return price_.compare_exchange_strong(oldPrice, newPrice, 
                                            std::memory_order_relaxed);
        }

        bool updateVolumeIfChanged(double newVolume) {
            double oldVolume = volume_.load(std::memory_order_relaxed);
            return volume_.compare_exchange_strong(oldVolume, newVolume, 
                                                std::memory_order_relaxed);
        }

        // 벌크 처리를 위한 배치 메서드(Repository 레이어로 이동 예정.)
        //static void process_updates(size_t batch_size = 100);
        //static void process_batch(const std::vector<std::shared_ptr<MarketData>>& batch);

    private:
        std::atomic<int64_t> id_{0};
        char symbol_[16];  // 고정 크기 배열로 변경
        std::atomic<double> price_{0.0};
        std::atomic<double> volume_{0.0};
        trantor::Date timestamp_;
        char source_[32];  // 고정 크기 배열로 변경
        trantor::Date created_at_;

        // 메모리 풀 싱글톤
        static memory::MemoryPool<MarketData>& memory_pool();
        // Queue 관련 멤버
        static containers::LockFreeQueue<std::shared_ptr<MarketData>>& update_queue();
    };
}