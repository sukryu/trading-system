#pragma once

#include <cds/opt/options.h>
#include <cstddef>
#include <chrono>

namespace containers {

    // 기본 컨테이너 특성 정의
    template<typename T>
    struct ContainerTraits {
        // 메모리 관리
        static constexpr size_t default_block_size = 256;
        static constexpr size_t max_cached_blocks = 1024;
        static constexpr size_t batch_size = 128;

        // 성능 설정
        static constexpr bool enable_statistics = true;
        static constexpr bool enable_backoff = true;
        static constexpr bool enable_memory_reclaim = true;

        // 모니터링
        static constexpr std::chrono::milliseconds stats_interval{1000};  // 1초
        
        // 백오프 설정
        static constexpr size_t min_backoff_delay = 1;
        static constexpr size_t max_backoff_delay = 1024;
        static constexpr size_t backoff_multiplier = 2;
    };

} // namespace containers