#include "memory/PoolMemoryResource.h"
#include <cassert>
#include <cstdlib>
#include <new>

namespace memory {

    void PoolMemoryResource::initialize() {
        const std::size_t actualBlockSize = sizeof(Block) - 1 + blockSize_;
        const std::size_t alignment = alignof(std::max_align_t);
        const std::size_t alignedBlockSize = (actualBlockSize + alignment - 1) & ~(alignment - 1);

        // 첫 번째 청크 할당
        std::size_t chunkSize = alignedBlockSize * (maxBlocks_ / 4); // 전체 크기의 1/4씩 할당
        auto* memory = static_cast<std::byte*>(std::aligned_alloc(alignment, chunkSize));
        if (!memory) {
            throw std::bad_alloc();
        }

        // 청크 저장
        chunks_.push_back({memory, chunkSize});

        // 블록들을 프리 리스트에 연결
        std::size_t numBlocks = chunkSize / alignedBlockSize;
        Block* current = reinterpret_cast<Block*>(memory);
        freeList_ = current;

        for (std::size_t i = 1; i < numBlocks; ++i) {
            auto* next = reinterpret_cast<Block*>(reinterpret_cast<std::byte*>(current) + alignedBlockSize);
            current->next = next;
            current = next;
        }
        current->next = nullptr;
    }

    void PoolMemoryResource::cleanup() {
        // 모든 청크 메모리 해제
        for (const auto& chunk : chunks_) {
            std::free(chunk.memory);
        }
        chunks_.clear();
        freeList_ = nullptr;
    }

    void* PoolMemoryResource::do_allocate(std::size_t bytes, std::size_t alignment) {
        if (bytes > blockSize_ || alignment > alignof(std::max_align_t)) {
            // 요청된 크기나 정렬이 처리할 수 없는 경우 폴백 리소스 사용
            return fallbackResource_.allocate(bytes, alignment);
        }

        std::lock_guard<std::mutex> lock(mutex_);

        // 프리 리스트가 비어있는 경우 새로운 청크 할당
        if (!freeList_) {
            const std::size_t actualBlockSize = sizeof(Block) - 1 + blockSize_;
            const std::size_t alignedBlockSize = (actualBlockSize + alignment - 1) & ~(alignment - 1);
            
            if (allocated_blocks() >= maxBlocks_) {
                throw std::bad_alloc();
            }

            std::size_t remainingBlocks = maxBlocks_ - allocated_blocks();
            std::size_t numNewBlocks = std::min(remainingBlocks, maxBlocks_ / 4);
            std::size_t chunkSize = alignedBlockSize * numNewBlocks;

            auto* memory = static_cast<std::byte*>(std::aligned_alloc(alignment, chunkSize));
            if (!memory) {
                throw std::bad_alloc();
            }

            chunks_.push_back({memory, chunkSize});

            // 새 블록들을 프리 리스트에 연결
            Block* current = reinterpret_cast<Block*>(memory);
            freeList_ = current;

            for (std::size_t i = 1; i < numNewBlocks; ++i) {
                auto* next = reinterpret_cast<Block*>(reinterpret_cast<std::byte*>(current) + alignedBlockSize);
                current->next = next;
                current = next;
            }
            current->next = nullptr;
        }

        // 프리 리스트에서 블록 할당
        Block* block = freeList_;
        freeList_ = block->next;
        allocatedBlocks_.fetch_add(1, std::memory_order_relaxed);

        return block->data;
    }

    void PoolMemoryResource::do_deallocate(void* p, std::size_t bytes, std::size_t alignment) {
        if (bytes > blockSize_ || alignment > alignof(std::max_align_t)) {
            // 폴백 리소스로 할당된 메모리 해제
            fallbackResource_.deallocate(p, bytes, alignment);
            return;
        }

        if (!p) return;

        // 블록을 프리 리스트에 반환
        Block* block = reinterpret_cast<Block*>(static_cast<std::byte*>(p) - offsetof(Block, data));
        
        std::lock_guard<std::mutex> lock(mutex_);
        block->next = freeList_;
        freeList_ = block;
        allocatedBlocks_.fetch_sub(1, std::memory_order_relaxed);
    }

    bool PoolMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
        return this == &other;
    }

} // namespace memory