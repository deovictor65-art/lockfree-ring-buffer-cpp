#include "lockfree/spsc_queue.hpp"
#include "lockfree/mpmc_queue.hpp"
#include <cassert>
#include <iostream>
#include <string>

void test_spsc_fifo_ordering() {
    constexpr size_t CAP = 16;
    lockfree::SPSCQueue<int, CAP> q;

    for (int i = 0; i < 10; ++i) {
        bool ok = q.push(i);
        assert(ok);
    }

    assert(q.size() == 10);

    for (int i = 0; i < 10; ++i) {
        int val = -1;
        bool ok = q.pop(val);
        assert(ok);
        assert(val == i);
    }

    assert(q.empty());
    std::cout << "[PASS] SPSC FIFO ordering and size tracking\n";
}

void test_mpmc_correctness() {
    constexpr size_t CAP = 32;
    lockfree::MPMCQueue<std::string, CAP> q;

    assert(q.push("HFT_QUOTE_1"));
    assert(q.push("HFT_QUOTE_2"));

    std::string val;
    assert(q.pop(val));
    assert(val == "HFT_QUOTE_1");
    assert(q.pop(val));
    assert(val == "HFT_QUOTE_2");
    assert(!q.pop(val)); // Empty
    std::cout << "[PASS] MPMC sequence ordering and boundary semantics\n";
}

int main() {
    std::cout << "Running Lock-Free Queues Unit Tests...\n";
    test_spsc_fifo_ordering();
    test_mpmc_correctness();
    std::cout << "\nAll Unit Tests Passed Successfully!\n";
    return 0;
}
