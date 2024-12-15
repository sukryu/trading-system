#pragma once

#include "containers/LockFreeContainers.h"

namespace containers {

    // Queue에 특화된 추가 기능 구현
    template<typename T>
    class LockFreeQueueImpl : public LockFreeQueue<T> {
    public:
        // 배치 처리를 위한 특화 구현
        template<typename OutputIt>
        size_t drain_to(OutputIt dest, size_t max_items) {
            size_t count = 0;
            auto start = std::chrono::steady_clock::now();

            while (count < max_items) {
                if (auto item = this->pop()) {
                    *dest++ = std::move(*item);
                    ++count;
                } else {
                    break;
                }
            }

            auto end = std::chrono::steady_clock::now();
            this->get_statistics().record_operation(
                "batch_pop",
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            );

            return count;
        }

        // 벌크 푸시 연산
        template<typename InputIt>
        size_t push_batch(InputIt first, InputIt last) {
            size_t count = 0;
            auto start = std::chrono::steady_clock::now();

            for (; first != last; ++first) {
                if (this->push(*first)) {
                    ++count;
                } else {
                    break;
                }
            }

            auto end = std::chrono::steady_clock::now();
            this->get_statistics().record_operation(
                "batch_push",
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
            );

            return count;
        }
    };
}