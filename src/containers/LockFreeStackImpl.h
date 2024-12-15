#pragma once
#include "containers/LockFreeContainers.h"

namespace containers {
    template<typename T>
    class LockFreeStackImpl : public LockFreeStack<T> {
    public:
        // 깊이 제한이 있는 푸시 연산
        bool push_with_depth_limit(T&& value, size_t max_depth) {
            if (depth_.load() >= max_depth) {
                return false;
            }

            if (this->push(std::move(value))) {
                depth_.fetch_add(1);
                return true;
            }
            return false;
        }

        // 스택 깊이 조회
        size_t get_depth() const {
            return depth_.load();
        }

        // 스택 내용을 다른 스택으로 이동
        void transfer_to(LockFreeStackImpl& other) {
            while (auto item = this->pop()) {
                other.push(std::move(*item));
            }
        }

    private:
        std::atomic<size_t> depth_{0};
    };
}