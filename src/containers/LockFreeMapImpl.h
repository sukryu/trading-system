#pragma once

#include "containers/LockFreeContainers.h"

namespace containers {
    template<typename Key, typename Value>
    class LockFreeMapImpl : public LockFreeMap<Key, Value> {
    public:
        // 범위 검색 구현
        template<typename K>
        std::vector<std::pair<Key, Value>> range_query(const K& start, const K& end) {
            std::vector<std::pair<Key, Value>> result;
            auto start_time = std::chrono::steady_clock::now();

            this->for_each([&](const Key& key, const Value& value) {
                if (key >= start && key <= end) {
                    result.emplace_back(key, value);
                }
            });

            auto end_time = std::chrono::steady_clock::now();
            this->get_statistics().record_operation(
                "range_query",
                std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time)
            );

            return result;
        }

        // 원자적 업데이트 (CAS 기반)
        template<typename UpdateFn>
        bool update_if(const Key& key, UpdateFn update_fn) {
            auto start_time = std::chrono::steady_clock::now();
            bool success = false;

            while (!success) {
                if (auto current = this->find(key)) {
                    Value new_value = update_fn(*current);
                    success = this->compare_exchange(key, *current, new_value);
                } else {
                    break;
                }
            }

            auto end_time = std::chrono::steady_clock::now();
            this->get_statistics().record_operation(
                "atomic_update",
                std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time)
            );

            return success;
        }

        // 벌크 삭제
        template<typename Predicate>
        size_t bulk_erase(Predicate pred) {
            size_t count = 0;
            auto start_time = std::chrono::steady_clock::now();

            std::vector<Key> to_erase;
            this->for_each([&](const Key& key, const Value& value) {
                if (pred(key, value)) {
                    to_erase.push_back(key);
                }
            });

            for (const auto& key : to_erase) {
                if (this->erase(key)) {
                    ++count;
                }
            }

            auto end_time = std::chrono::steady_clock::now();
            this->get_statistics().record_operation(
                "bulk_erase",
                std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time)
            );

            return count;
        }
    };
}