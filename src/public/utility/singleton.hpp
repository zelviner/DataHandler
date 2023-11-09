#pragma once

#include <atomic>
#include <mutex>

namespace zel {
namespace utility {

template <typename T> class Singleton {
  public:
    static T *instance() {
        T *instance = instance_.load(std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_acquire);
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            instance = instance_.load(std::memory_order_relaxed);
            if (instance == nullptr) {
                instance = new T();
                std::atomic_thread_fence(std::memory_order_release);
                instance_.store(instance, std::memory_order_relaxed);
            }
        }
        return instance;
    }

  private:
    Singleton()                             = default;
    Singleton(const Singleton &)            = delete;
    Singleton &operator=(const Singleton &) = delete;
    ~Singleton()                            = default;

    static std::atomic<T *> instance_;
    static std::mutex       mutex_;
};

template <typename T> std::atomic<T *> Singleton<T>::instance_{nullptr};

template <typename T> std::mutex Singleton<T>::mutex_;

} // namespace utility

} // namespace zel