#pragma once

#include <cstddef>

namespace common {

    struct MemoryConfig {
        static constexpr std::size_t DEFAULT_BLOCK_SIZE = 64;
        static constexpr std::size_t DEFAULT_POOL_SIZE = 1024;
        static constexpr std::size_t CACHE_LINE_SIZE = 64;
        static constexpr std::size_t PAGE_SIZE = 4096;
    };

    struct QueueConfig {
        static constexpr std::size_t DEFAULT_QUEUE_SIZE = 1024;
        static constexpr std::size_t MAX_THREADS = 32;
    };

} // namespace common