#pragma once

#include "LockFreeQueueImpl.h"
#include "LockFreeStackImpl.h"
#include "LockFreeMapImpl.h"

namespace containers {
    class LockFreeContainerFactory {
    public:
        template<typename T>
        static std::unique_ptr<LockFreeQueueImpl<T>> create_queue() {
            return std::make_unique<LockFreeQueueImpl<T>>();
        }

        template<typename T>
        static std::unique_ptr<LockFreeStackImpl<T>> create_stack() {
            return std::make_unique<LockFreeStackImpl<T>>();
        }

        template<typename Key, typename Value>
        static std::unique_ptr<LockFreeMapImpl<Key, Value>> create_map() {
            return std::make_unique<LockFreeMapImpl<Key, Value>>();
        }
    };
}