#pragma once

#include <vector>
#include <memory>
#include "ContainerTraits.h"

namespace containers {

    template<typename T>
    class MemoryManager {
    public:
        MemoryManager() {
            if (ContainerTraits<T>::enable_memory_reclaim) {
                free_blocks_.reserve(ContainerTraits<T>::max_cached_blocks);
            }
        }

        void* allocate(size_t size) {
            if (!ContainerTraits<T>::enable_memory_reclaim || free_blocks_.empty()) {
                return ::operator new(size);
            }
            void* ptr = free_blocks_.back();
            free_blocks_.pop_back();
            return ptr;
        }

        void deallocate(void* ptr) {
            if (!ContainerTraits<T>::enable_memory_reclaim || 
                free_blocks_.size() >= ContainerTraits<T>::max_cached_blocks) {
                ::operator delete(ptr);
                return;
            }
            free_blocks_.push_back(ptr);
        }

    private:
        std::vector<void*> free_blocks_;
    };

} // namespace containers