#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    const std::vector<size_t> sizes = {1024, 16384, 262144, 1048576};

    std::cout << "memory_pressure_sim\n";
    std::cout << "items,elapsed_us,checksum\n";

    for (size_t n : sizes) {
        std::vector<int> data(n);
        std::iota(data.begin(), data.end(), 1);

        volatile long long checksum = 0;
        auto start = std::chrono::steady_clock::now();

        for (size_t i = 0; i < data.size(); i += 16) {
            checksum += data[i];
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count();

        std::cout << n << "," << elapsed << "," << checksum << "\n";
    }

    return 0;
}
