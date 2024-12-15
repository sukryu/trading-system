#pragma once

#include <atomic>
#include <chrono>
#include "ContainerTraits.h"

namespace containers {

    template<typename T>
    class ContainerStatistics {
    public:
        void record_operation(const std::string& op_type, std::chrono::nanoseconds duration) {
            if (ContainerTraits<T>::enable_statistics) {
                ++operation_count_;
                total_duration_ += duration.count();
                if (op_type == "push") ++push_count_;
                else if (op_type == "pop") ++pop_count_;
            }
        }

        void record_contention() {
            if (ContainerTraits<T>::enable_statistics) {
                ++contention_count_;
            }
        }

        // 통계 조회
        size_t get_operation_count() const { return operation_count_; }
        size_t get_push_count() const { return push_count_; }
        size_t get_pop_count() const { return pop_count_; }
        size_t get_contention_count() const { return contention_count_; }
        double get_average_duration() const {
            return operation_count_ > 0 
                ? static_cast<double>(total_duration_) / operation_count_ 
                : 0.0;
        }

        void reset() {
            operation_count_ = 0;
            push_count_ = 0;
            pop_count_ = 0;
            contention_count_ = 0;
            total_duration_ = 0;
        }

    private:
        std::atomic<size_t> operation_count_{0};
        std::atomic<size_t> push_count_{0};
        std::atomic<size_t> pop_count_{0};
        std::atomic<size_t> contention_count_{0};
        std::atomic<int64_t> total_duration_{0};
    };

} // namespace containers