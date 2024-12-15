// include/memory/GarbageCollector.h
#pragma once

#include <cds/gc/hp.h>
#include <memory>

namespace memory {

    class GarbageCollector {
    public:
        static void attach_thread() {
            // 현재 스레드를 GC에 연결
            if (cds::threading::Manager::isThreadAttached())
                return;
            cds::threading::Manager::attachThread();
        }

        static void detach_thread() {
            // 현재 스레드를 GC에서 분리
            if (!cds::threading::Manager::isThreadAttached())
                return;
            cds::threading::Manager::detachThread();
        }

        class ThreadGuard {
        public:
            ThreadGuard() { attach_thread(); }
            ~ThreadGuard() { detach_thread(); }
        };
    };

} // namespace memory