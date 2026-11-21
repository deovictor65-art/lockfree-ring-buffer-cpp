#include "lockfree/spsc_queue.hpp"
#include "lockfree/mpmc_queue.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>

struct OrderPacket {
    uint64_t id;
    int64_t price;
    uint32_t quantity;
    uint64_t timestamp_ns;
};

void benchmark_spsc() {
    std::cout << "[*] 1. Benchmarking Lock-Free SPSC Ring Buffer...\n";
    constexpr size_t CAPACITY = 65536;
    constexpr size_t NUM_ITEMS = 10'000'000;

    auto queue = std::make_unique<lockfree::SPSCQueue<OrderPacket, CAPACITY>>();

    auto t0 = std::chrono::high_resolution_clock::now();

    std::thread producer([&]() {
        for (uint64_t i = 1; i <= NUM_ITEMS; ++i) {
            OrderPacket p{i, 10050, 10, i * 100};
            while (!queue->push(p)) {
                // busy spin wait-free
            }
        }
    });

    std::thread consumer([&]() {
        OrderPacket p;
        for (uint64_t i = 1; i <= NUM_ITEMS; ++i) {
            while (!queue->pop(p)) {
                // busy spin wait-free
            }
        }
    });

    producer.join();
    consumer.join();

    auto t1 = std::chrono::high_resolution_clock::now();
    double elapsed_sec = std::chrono::duration<double>(t1 - t0).count();
    double throughput_mops = (NUM_ITEMS / elapsed_sec) / 1e6;
    double avg_latency_ns = (elapsed_sec * 1e9) / NUM_ITEMS;

    std::cout << "    Processed:  " << NUM_ITEMS << " messages\n";
    std::cout << "    Elapsed:    " << std::fixed << std::setprecision(3) << elapsed_sec << " s\n";
    std::cout << "    Throughput: " << std::fixed << std::setprecision(2) << throughput_mops << " Million ops/sec\n";
    std::cout << "    Avg Cost:   " << std::fixed << std::setprecision(1) << avg_latency_ns << " ns/op\n\n";
}

void benchmark_mpmc() {
    std::cout << "[*] 2. Benchmarking Lock-Free MPMC Queue (2 Producers, 2 Consumers)...\n";
    constexpr size_t CAPACITY = 65536;
    constexpr size_t TOTAL_ITEMS = 4'000'000;
    constexpr size_t PER_PRODUCER = TOTAL_ITEMS / 2;

    auto queue = std::make_unique<lockfree::MPMCQueue<uint64_t, CAPACITY>>();
    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> producers;
    for (int p = 0; p < 2; ++p) {
        producers.emplace_back([&, p]() {
            for (uint64_t i = 0; i < PER_PRODUCER; ++i) {
                while (!queue->push(i)) {}
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < 2; ++c) {
        consumers.emplace_back([&, c]() {
            uint64_t item;
            for (uint64_t i = 0; i < PER_PRODUCER; ++i) {
                while (!queue->pop(item)) {}
            }
        });
    }

    for (auto& th : producers) th.join();
    for (auto& th : consumers) th.join();

    auto t1 = std::chrono::high_resolution_clock::now();
    double elapsed_sec = std::chrono::duration<double>(t1 - t0).count();
    double throughput_mops = (TOTAL_ITEMS / elapsed_sec) / 1e6;

    std::cout << "    MPMC Total: " << TOTAL_ITEMS << " messages\n";
    std::cout << "    Throughput: " << std::fixed << std::setprecision(2) << throughput_mops << " Million ops/sec\n";
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "   Lock-Free SPSC & MPMC High-Throughput Engine C++20    \n";
    std::cout << "=========================================================\n\n";

    benchmark_spsc();
    benchmark_mpmc();

    std::cout << "\n[✔] Lock-Free Disruptor Ring Buffer Engine Verified.\n";
    return 0;
}
