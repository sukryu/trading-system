#pragma once

#include <cds/container/msqueue.h>
#include <cds/container/treiber_stack.h>
#include <cds/container/skip_list_map_hp.h>
#include <optional>
#include "ContainerTraits.h"
#include "BackoffStrategy.h"
#include "Statistics.h"
#include "MemoryManager.h"

namespace containers {
    template<typename T>
    class LockFreeQueue {
    public:
        using value_type = T;
        using traits_type = ContainerTraits<T>;
        using statistics_type = ContainerStatistics<T>;
        using backoff_type = ExponentialBackoff<T>;
        using memory_manager_type = MemoryManager<T>;

        struct queue_traits : public cds::container::msqueue::traits {
            using backoff_strategy = typename traits_type::BackoffStrategy::type;
            static constexpr size_t allocation_size = traits_type::default_block_size;
        }

        using queue_type = cds::container::MSQueue<cds::gc::HP, T, queue_traits>;

        LockFreeQueue() : statistics_{}, memory_manager_{} {
            cds::threading::Manager::attachThread();
        }

        ~LockFreeQueue() {
            cds::threading::Manager::detachThread();
        }

        template<typename... Args>
        bool emplace(Args&&... args) {
            auto start = std::chrono::steady_clock::now();
            bool result = false;
            
            void* ptr = memory_manager_.allocate(sizeof(T));
            try {
                new(ptr) T(std::forward<Args>(args)...);
                result = queue_.enqueue(*static_cast<T*>(ptr));
                if (!result) {
                    static_cast<T*>(ptr)->~T();
                    memory_manager_.deallocate(ptr);
                    if (traits_type::enable_backoff) {
                        backoff_();
                        statistics_.record_contention();
                    }
                }
            } catch (...) {
                memory_manager_.deallocate(ptr);
                throw;
            }

            auto end = std::chrono::steady_clock::now();
            statistics_.record_operation("push", end - start);
            return result;
        }

        bool push(const T& value) {
        return emplace(value);
        }

        bool push(T&& value) {
            return emplace(std::move(value));
        }

        std::optional<T> pop() {
            auto start = std::chrono::steady_clock::now();
            T value;
            bool success = queue_.dequeue(value);
            
            if (!success && traits_type::enable_backoff) {
                backoff_();
                statistics_.record_contention();
            }

            auto end = std::chrono::steady_clock::now();
            statistics_.record_operation("pop", end - start);

            if (success) {
                return std::make_optional(std::move(value));
            }
            return std::nullopt;
        }

        // 배치 처리 메서드
        template<typename OutputIterator>
        size_t pop_batch(OutputIterator out, size_t max_items) {
            size_t count = 0;
            while (count < max_items) {
                if (auto item = pop()) {
                    *out++ = std::move(*item);
                    ++count;
                } else {
                    break;
                }
            }
            return count;
        }

        // 통계 메서드
        const statistics_type& get_statistics() const { return statistics_; }
        void reset_statistics() { statistics_.reset(); }

        bool empty() const { return queue_.empty(); }

    private:
        queue_type queue_;
        statistics_type statistics_;
        memory_manager_type memory_manager_;
        backoff_type backoff_;
    };

    template<typename T>
    class LockFreeStack {
    public:
        using value_type = T;
        using traits_type = ContainerTraits<T>;
        using statistics_type = ContainerStatistics<T>;
        using backoff_type = ExponentialBackoff<T>;
        using memory_manager_type = MemoryManager<T>;

        struct stack_traits : public cds::container::treiber_stack::traits {
            using backoff_strategy = typename traits_type::BackoffStrategy::type;
            static constexpr size_t allocation_size = traits_type::default_block_size;
        };

        using stack_type = cds::container::TreiberStack<cds::gc::HP, T, stack_traits>;

        LockFreeStack() : statistics_{}, memory_manager_{} {
            cds::threading::Manager::attachThread();
        }

        ~LockFreeStack() {
            cds::threading::Manager::detachThread();
        }

        template<typename... Args>
        bool emplace(Args&&... args) {
            auto start = std::chrono::steady_clock::now();
            bool result = false;
            
            void* ptr = memory_manager_.allocate(sizeof(T));
            try {
                new(ptr) T(std::forward<Args>(args)...);
                result = stack_.push(*static_cast<T*>(ptr));
                if (!result) {
                    static_cast<T*>(ptr)->~T();
                    memory_manager_.deallocate(ptr);
                    if (traits_type::enable_backoff) {
                        backoff_();
                        statistics_.record_contention();
                    }
                }
            } catch (...) {
                memory_manager_.deallocate(ptr);
                throw;
            }

            auto end = std::chrono::steady_clock::now();
            statistics_.record_operation("push", end - start);
            return result;
        }

        bool push(const T& value) {
            return emplace(value);
        }

        bool push(T&& value) {
            return emplace(std::move(value));
        }

        std::optional<T> pop() {
            auto start = std::chrono::steady_clock::now();
            T value;
            bool success = stack_.pop(value);
            
            if (!success && traits_type::enable_backoff) {
                backoff_();
                statistics_.record_contention();
            }

            auto end = std::chrono::steady_clock::now();
            statistics_.record_operation("pop", end - start);

            if (success) {
                return std::make_optional(std::move(value));
            }
            return std::nullopt;
        }

        const statistics_type& get_statistics() const { return statistics_; }
        void reset_statistics() { statistics_.reset(); }

        bool empty() const { return stack_.empty(); }

    private:
        stack_type stack_;
        statistics_type statistics_;
        memory_manager_type memory_manager_;
        backoff_type backoff_;
    };

    template<typename Key, typename Value>
    class LockFreeMap {
    public:
        using key_type = Key;
        using mapped_type = Value;
        using traits_type = ContainerTraits<Key>;
        using statistics_type = ContainerStatistics<Key>;
        using backoff_type = ExponentialBackoff<Key>;
        using memory_manager_type = MemoryManager<std::pair<const Key, Value>>;

        struct map_traits : public cds::container::skip_list::traits {
            using backoff_strategy = typename traits_type::BackoffStrategy::type;
            static constexpr size_t allocation_size = traits_type::default_block_size;
        };

        using map_type = cds::container::SkipListMap<cds::gc::HP, Key, Value, map_traits>;

        LockFreeMap() : statistics_{}, memory_manager_{} {
            cds::threading::Manager::attachThread();
        }

        ~LockFreeMap() {
            cds::threading::Manager::detachThread();
        }

        bool insert(const Key& key, const Value& value) {
            auto start = std::chrono::steady_clock::now();
            bool success = map_.insert(key, value);
            
            if (!success && traits_type::enable_backoff) {
                backoff_();
                statistics_.record_contention();
            }

            auto end = std::chrono::steady_clock::now();
            statistics_.record_operation("insert", end - start);
            return success;
        }

        bool erase(const Key& key) {
            auto start = std::chrono::steady_clock::now();
            bool success = map_.erase(key);
            
            if (!success && traits_type::enable_backoff) {
                backoff_();
                statistics_.record_contention();
            }

            auto end = std::chrono::steady_clock::now();
            statistics_.record_operation("erase", end - start);
            return success;
        }

        std::optional<Value> find(const Key& key) const {
            auto start = std::chrono::steady_clock::now();
            Value value;
            bool success = map_.find(key, value);
            
            if (!success && traits_type::enable_backoff) {
                backoff_();
                statistics_.record_contention();
            }

            auto end = std::chrono::steady_clock::now();
            statistics_.record_operation("find", end - start);

            if (success) {
                return std::make_optional(std::move(value));
            }
            return std::nullopt;
        }

        template<typename F>
        void for_each(F&& f) const {
            map_.forEach(std::forward<F>(f));
        }

        const statistics_type& get_statistics() const { return statistics_; }
        void reset_statistics() { statistics_.reset(); }

        bool empty() const { return map_.empty(); }

    private:
        map_type map_;
        statistics_type statistics_;
        memory_manager_type memory_manager_;
        mutable backoff_type backoff_;
    };
} // namespace containers

