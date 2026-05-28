#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main() {
    constexpr int threads = 4;
    constexpr int iterations = 50000;

    int unsafe_counter = 0;
    int locked_counter = 0;
    std::atomic<int> atomic_counter{0};
    std::mutex m;

    auto unsafe_work = [&]() {
        for (int i = 0; i < iterations; ++i) {
            unsafe_counter++;
        }
    };

    auto locked_work = [&]() {
        for (int i = 0; i < iterations; ++i) {
            std::lock_guard<std::mutex> lock(m);
            locked_counter++;
        }
    };

    auto atomic_work = [&]() {
        for (int i = 0; i < iterations; ++i) {
            atomic_counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    auto run = [](auto fn) {
        std::vector<std::thread> workers;
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < threads; ++i) workers.emplace_back(fn);
        for (auto& t : workers) t.join();
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count();
    };

    auto unsafe_us = run(unsafe_work);
    auto locked_us = run(locked_work);
    auto atomic_us = run(atomic_work);

    const int expected = threads * iterations;

    std::cout << "expected=" << expected << "\n";
    std::cout << "unsafe_counter=" << unsafe_counter << "\n";
    std::cout << "locked_counter=" << locked_counter << "\n";
    std::cout << "atomic_counter=" << atomic_counter.load() << "\n";
    std::cout << "unsafe_us=" << unsafe_us << "\n";
    std::cout << "locked_us=" << locked_us << "\n";
    std::cout << "atomic_us=" << atomic_us << "\n";

    return (locked_counter == expected && atomic_counter.load() == expected) ? 0 : 1;
}
