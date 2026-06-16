#pragma once
#include <cstdint>
#include <vector>

namespace firmware_validation {

class HardwareInterfaceMock {
public:
    void write_register(uint32_t address, uint32_t value) {
        writes_.push_back({address, value});
    }

    uint32_t read_register(uint32_t address) const {
        for (auto it = writes_.rbegin(); it != writes_.rend(); ++it) {
            if (it->address == address) {
                return it->value;
            }
        }
        return 0;
    }

    size_t write_count() const {
        return writes_.size();
    }

private:
    struct RegisterWrite {
        uint32_t address;
        uint32_t value;
    };

    std::vector<RegisterWrite> writes_;
};

}  // namespace firmware_validation
