#pragma once

#include <thread>
#include <chrono>
#include "ContainerTraits.h"

namespace containers {

    template<typename T>
    class ExponentialBackoff {
    public:
        ExponentialBackoff() : current_delay_(ContainerTraits<T>::min_backoff_delay) {}

        void operator()() {
            if (current_delay_ < ContainerTraits<T>::max_backoff_delay) {
                std::this_thread::sleep_for(std::chrono::microseconds(current_delay_));
                current_delay_ *= ContainerTraits<T>::backoff_multiplier;
            } else {
                std::this_thread::yield();
            }
        }

        void reset() {
            current_delay_ = ContainerTraits<T>::min_backoff_delay;
        }

    private:
        size_t current_delay_;
    };

} // namespace containers