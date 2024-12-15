#pragma once

#include <memory_resource>
#include <array>
#include <mutex>
#include <vector>
#include <cstddef>
#include <atomic>

namespace memory {

    class PoolMemoryResource : public std::pmr::memory_resource {
    public:
        explicit PoolMemoryResource(std::size_t blockSize, std::size_t maxBlocks = 1024)
            : blockSize_(blockSize)
            , maxBlocks_(maxBlocks)
            , allocatedBlocks_(0) {
            initialize();
        }

        ~PoolMemoryResource() override {
            cleanup();
        }

        // 현재 할당된 블록 수 반환
        std::size_t allocated_blocks() const noexcept {
            return allocatedBlocks_.load(std::memory_order_relaxed);
        }

        // 최대 블록 수 반환
        std::size_t max_blocks() const noexcept {
            return maxBlocks_;
        }

        // 블록 크기 반환
        std::size_t block_size() const noexcept {
            return blockSize_;
        }

    private:
        void* do_allocate(std::size_t bytes, std::size_t alignment) override;
        void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override;
        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

        void initialize();
        void cleanup();

        // 메모리 블록 구조체
        struct alignas(std::max_align_t) Block {
            Block* next;
            std::byte data[1];  // 실제 크기는 동적으로 결정
        };

        // 메모리 청크 (대량의 블록을 한번에 할당)
        struct Chunk {
            std::byte* memory;
            std::size_t size;
        };

        static Block* to_block(void* p) noexcept {
            return static_cast<Block*>(p);
        }

        std::size_t blockSize_;            // 각 블록의 크기
        std::size_t maxBlocks_;           // 최대 블록 수
        std::atomic<std::size_t> allocatedBlocks_; // 현재 할당된 블록 수
        Block* freeList_{nullptr};        // 사용 가능한 블록 리스트
        std::vector<Chunk> chunks_;       // 할당된 청크들
        std::mutex mutex_;                // 동기화를 위한 뮤텍스
        std::pmr::unsynchronized_pool_resource fallbackResource_; // 폴백 리소스
    };

} // namespace memory