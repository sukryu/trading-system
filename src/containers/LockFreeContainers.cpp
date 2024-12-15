#include "containers/LockFreeContainers.h"
#include "utils/Logger.h"
#include <cassert>
#include <chrono>
#include <map>
#include <cds/init.h>           // Initialize, Terminate 함수용
#include <cds/gc/hp.h>          // Hazard pointer GC
#include <cds/container/msqueue.h>
#include <cds/container/treiber_stack.h>
#include <cds/container/skip_list_map_hp.h>
#include <cds/threading/model.h>  // threading::Manager

namespace containers {

    namespace {

    // libcds 초기화 및 정리를 관리하는 싱글톤
    class CdsRuntimeManager {
    public:
        static CdsRuntimeManager& instance() {
            static CdsRuntimeManager inst;
            return inst;
        }

        void initialize() {
            if (!initialized_) {
                cds::Initialize();
                TRADING_LOG_INFO("CDS runtime initialized");
                initialized_ = true;
            }
        }

        void terminate() {
            if (initialized_) {
                cds::Terminate();
                TRADING_LOG_INFO("CDS runtime terminated");
                initialized_ = false;
            }
        }

        bool is_initialized() const { return initialized_; }

    private:
        CdsRuntimeManager() {
            initialize();
        }
        
        ~CdsRuntimeManager() {
            terminate();
        }

        bool initialized_{false};
    };

    // 성능 측정을 위한 유틸리티 클래스
    class PerformanceMonitor {
    public:
        static void record_operation(const std::string& container_type,
                                const std::string& operation,
                                std::chrono::nanoseconds duration) {
            auto& inst = instance();
            inst.record(container_type, operation, duration);
        }

        static void report_statistics() {
            auto& inst = instance();
            inst.report();
        }

    private:
        static PerformanceMonitor& instance() {
            static PerformanceMonitor inst;
            return inst;
        }

        void record(const std::string& container_type,
                    const std::string& operation,
                    std::chrono::nanoseconds duration) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto& stats = statistics_[container_type][operation];
            stats.count++;
            stats.total_duration += duration;
            stats.max_duration = std::max(stats.max_duration, duration);
            stats.min_duration = std::min(stats.min_duration, duration);
        }

        void report() {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [container, ops] : statistics_) {
                for (const auto& [op, stats] : ops) {
                    TRADING_LOG_INFO(
                        "{} - {} operations: count={}, avg={}ns, min={}ns, max={}ns",
                        container, op, stats.count,
                        stats.count > 0 ? stats.total_duration.count() / stats.count : 0,
                        stats.min_duration.count(),
                        stats.max_duration.count()
                    );
                }
            }
        }

        struct OperationStats {
            size_t count{0};
            std::chrono::nanoseconds total_duration{0};
            std::chrono::nanoseconds max_duration{0};
            std::chrono::nanoseconds min_duration{std::chrono::nanoseconds::max()};
        };

        std::mutex mutex_;
        std::map<std::string, std::map<std::string, OperationStats>> statistics_;
    };

    } // anonymous namespace

    // 전역 초기화 및 정리 함수
    void initialize_lock_free_containers() {
        CdsRuntimeManager::instance().initialize();
    }

    void terminate_lock_free_containers() {
        PerformanceMonitor::report_statistics();
        CdsRuntimeManager::instance().terminate();
    }

    // 성능 모니터링을 위한 래퍼 함수들
    template<typename Func>
    auto measure_operation(const std::string& container_type,
                        const std::string& operation,
                        Func&& func) {
        auto start = std::chrono::steady_clock::now();
        auto result = std::forward<Func>(func)();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        PerformanceMonitor::record_operation(container_type, operation, duration);
        return result;
    }

    // 스레드 로컬 저장소를 사용한 GC 관리
    thread_local struct ThreadGcGuard {
        ThreadGcGuard() {
            cds::threading::Manager::attachThread();
            TRADING_LOG_DEBUG("Thread attached to CDS runtime");
        }
        
        ~ThreadGcGuard() {
            cds::threading::Manager::detachThread();
            TRADING_LOG_DEBUG("Thread detached from CDS runtime");
        }
    } gc_guard;

    // Hazard Pointer 관리를 위한 유틸리티 클래스
    class HazardPointerGuard {
    public:
        template<typename T>
        HazardPointerGuard(const T* ptr) {
            hp_ = cds::gc::HP::Guard::allocGuard();
            hp_->assign(ptr);
        }
        
        ~HazardPointerGuard() {
            hp_->clear();
        }

    private:
        cds::gc::HP::Guard* hp_;
    };

    // 성능 분석을 위한 진단 함수들
    void analyze_container_performance(const std::string& container_type,
                                    const ContainerStatistics<void>& stats) {
        TRADING_LOG_INFO("{} Performance Analysis:", container_type);
        TRADING_LOG_INFO("Total operations: {}", stats.get_operation_count());
        TRADING_LOG_INFO("Push operations: {}", stats.get_push_count());
        TRADING_LOG_INFO("Pop operations: {}", stats.get_pop_count());
        TRADING_LOG_INFO("Contention count: {}", stats.get_contention_count());
        TRADING_LOG_INFO("Average operation duration: {}ns", stats.get_average_duration());
    }

    // 메모리 사용량 진단
    void analyze_memory_usage(const std::string& container_type,
                            const MemoryManager<void>& memory_manager) {
        TRADING_LOG_INFO("{} Memory Usage:", container_type);
        // 메모리 사용량 분석 로직 구현
    }

} // namespace containers