#pragma once

#include <atomic>
#include <cstddef>
#include <new>
#include <utility>
#include <concepts>
#include <vector>

namespace lockfree {

#if defined(__cpp_lib_hardware_interference_size)
    constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
    constexpr size_t CACHE_LINE_SIZE = 64;
#endif

template <typename T, size_t Capacity>
requires ((Capacity & (Capacity - 1)) == 0 && Capacity > 0)
class SPSCQueue {
public:
    SPSCQueue() : buffer_(static_cast<T*>(::operator new(sizeof(T) * Capacity))) {}

    ~SPSCQueue() {
        T item;
        while (pop(item));
        ::operator delete(buffer_);
    }

    // Disable copy and move
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    template <typename... Args>
    bool emplace(Args&&... args) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        if (current_tail - cached_head_ >= Capacity) {
            cached_head_ = head_.load(std::memory_order_acquire);
            if (current_tail - cached_head_ >= Capacity) {
                return false; // Queue full
            }
        }

        new (&buffer_[current_tail & MASK]) T(std::forward<Args>(args)...);
        tail_.store(current_tail + 1, std::memory_order_release);
        return true;
    }

    bool push(const T& item) {
        return emplace(item);
    }

    bool push(T&& item) {
        return emplace(std::move(item));
    }

    bool pop(T& item) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        if (current_head == cached_tail_) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (current_head == cached_tail_) {
                return false; // Queue empty
            }
        }

        T* slot = &buffer_[current_head & MASK];
        item = std::move(*slot);
        slot->~T();
        head_.store(current_head + 1, std::memory_order_release);
        return true;
    }

    bool empty() const noexcept {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

    size_t size() const noexcept {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t t = tail_.load(std::memory_order_relaxed);
        return t >= h ? t - h : 0;
    }

private:
    static constexpr size_t MASK = Capacity - 1;

    alignas(CACHE_LINE_SIZE) std::atomic<size_t> tail_{0};
    alignas(CACHE_LINE_SIZE) size_t cached_head_{0};

    alignas(CACHE_LINE_SIZE) std::atomic<size_t> head_{0};
    alignas(CACHE_LINE_SIZE) size_t cached_tail_{0};

    alignas(CACHE_LINE_SIZE) T* buffer_;
};

} // namespace lockfree
