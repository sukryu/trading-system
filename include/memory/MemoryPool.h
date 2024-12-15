#pragma once

#include <memory>
#include <type_traits>
#include <memory_resource>
#include "PoolMemoryResource.h"

namespace memory {

    template<typename T>
    class MemoryPool {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using size_type = std::size_t;

        explicit MemoryPool(size_type initialSize = 1024)
            : memResource_(sizeof(T), initialSize)
            , allocator_(&memResource_) 
        {
            static_assert(std::is_destructible_v<T>, "T must be destructible");
        }

        // 복사 생성자와 할당 연산자 삭제
        MemoryPool(const MemoryPool&) = delete;
        MemoryPool& operator=(const MemoryPool&) = delete;

        // 이동 생성자와 할당 연산자
        MemoryPool(MemoryPool&&) noexcept = default;
        MemoryPool& operator=(MemoryPool&&) noexcept = default;

        // 단일 객체 할당
        template<typename... Args>
        pointer allocate(Args&&... args) {
            void* ptr = allocator_.allocate(1);
            try {
                return new(ptr) T(std::forward<Args>(args)...);
            } catch (...) {
                allocator_.deallocate(static_cast<T*>(ptr), 1);
                throw;
            }
        }

        // 배열 할당
        pointer allocate_array(size_type n) {
            if (n == 0) return nullptr;
            
            // 배열 크기를 저장할 추가 공간 할당
            void* ptr = allocator_.allocate(n + 1);
            if (!ptr) throw std::bad_alloc();

            // 배열 크기 저장
            *static_cast<size_type*>(ptr) = n;
            
            // 객체 배열의 시작 포인터
            T* objects = reinterpret_cast<T*>(static_cast<size_type*>(ptr) + 1);

            try {
                // 객체들을 기본 생성
                for (size_type i = 0; i < n; ++i) {
                    new(objects + i) T();
                }
                return objects;
            } catch (...) {
                allocator_.deallocate(static_cast<T*>(ptr), n + 1);
                throw;
            }
        }

        // 단일 객체 해제
        void deallocate(pointer p) noexcept {
            if (!p) return;
            p->~T();
            allocator_.deallocate(p, 1);
        }

        // 배열 해제
        void deallocate_array(pointer p) noexcept {
            if (!p) return;

            // 배열 크기 획득
            size_type* size_ptr = reinterpret_cast<size_type*>(p) - 1;
            size_type n = *size_ptr;

            // 각 객체 소멸
            for (size_type i = 0; i < n; ++i) {
                p[i].~T();
            }

            // 메모리 해제
            allocator_.deallocate(size_ptr, n + 1);
        }

        // 현재 할당된 블록 수 반환
        size_type allocated_blocks() const noexcept {
            return memResource_.allocated_blocks();
        }

        // 최대 블록 수 반환
        size_type max_blocks() const noexcept {
            return memResource_.max_blocks();
        }

    private:
        PoolMemoryResource memResource_;
        std::pmr::polymorphic_allocator<T> allocator_;
    };

    // 편의를 위한 make_unique_from_pool 함수 템플릿
    template<typename T, typename Pool, typename... Args>
    std::unique_ptr<T, typename Pool::Deleter> make_unique_from_pool(Pool& pool, Args&&... args) {
        return pool.template make_unique<T>(std::forward<Args>(args)...);
    }

} // namespace memory