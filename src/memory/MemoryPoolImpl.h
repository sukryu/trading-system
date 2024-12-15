#pragma once

#include "memory/MemoryPool.h"
#include <cassert>

namespace memory {

    namespace detail {

    // 메모리 정렬 헬퍼
    template<typename T>
    constexpr size_t calculate_aligned_size(size_t size) noexcept {
        constexpr size_t alignment = alignof(T);
        return (size + alignment - 1) & ~(alignment - 1);
    }

    // 예외 안전성을 위한 RAII 가드
    template<typename T>
    class ConstructGuard {
    public:
        explicit ConstructGuard(T* ptr) : ptr_(ptr), committed_(false) {}
        ~ConstructGuard() {
            if (!committed_) {
                ptr_->~T();
            }
        }
        void commit() { committed_ = true; }
    private:
        T* ptr_;
        bool committed_;
    };

    } // namespace detail

    // 특화된 할당자 인터페이스 구현
    template<typename T>
    template<typename... Args>
    typename MemoryPool<T>::pointer MemoryPool<T>::allocate(Args&&... args) {
        void* ptr = allocator_.allocate(1);
        pointer object = nullptr;

        try {
            object = new(ptr) T(std::forward<Args>(args)...);
            detail::ConstructGuard guard(object);
            // 추가적인 초기화가 필요한 경우 여기서 수행
            guard.commit();
            return object;
        } catch (...) {
            if (ptr) {
                allocator_.deallocate(static_cast<T*>(ptr), 1);
            }
            throw;
        }
    }

    template<typename T>
    typename MemoryPool<T>::pointer MemoryPool<T>::allocate_array(size_type n) {
        if (n == 0) return nullptr;

        // 메모리 레이아웃: [size][객체들...]
        const size_type total_size = detail::calculate_aligned_size<size_type>(sizeof(size_type)) + 
                                    n * sizeof(T);
        void* raw_ptr = allocator_.allocate(total_size);

        try {
            // 크기 정보 저장
            *static_cast<size_type*>(raw_ptr) = n;
            
            // 객체 배열의 시작 주소 계산
            pointer objects = reinterpret_cast<pointer>(
                static_cast<char*>(raw_ptr) + detail::calculate_aligned_size<size_type>(sizeof(size_type))
            );

            // 객체들 생성
            for (size_type i = 0; i < n; ++i) {
                new(objects + i) T();
            }

            return objects;
        } catch (...) {
            allocator_.deallocate(raw_ptr, total_size);
            throw;
        }
    }

} // namespace memory