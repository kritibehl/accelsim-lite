#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace accelsim {

enum class OpType {
    Compute,
    Load,
    Store
};

enum class Stage {
    Fetch,
    Decode,
    Dispatch,
    Ready,
    Execute,
    Retire,
    Done
};

inline std::string to_string(OpType op) {
    switch (op) {
        case OpType::Compute: return "COMPUTE";
        case OpType::Load: return "LOAD";
        case OpType::Store: return "STORE";
    }
    return "UNKNOWN";
}

inline std::string to_string(Stage stage) {
    switch (stage) {
        case Stage::Fetch: return "Fetch";
        case Stage::Decode: return "Decode";
        case Stage::Dispatch: return "Dispatch";
        case Stage::Ready: return "Ready";
        case Stage::Execute: return "Execute";
        case Stage::Retire: return "Retire";
        case Stage::Done: return "Done";
    }
    return "Unknown";
}

inline OpType parse_op_type(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (value == "COMPUTE") return OpType::Compute;
    if (value == "LOAD") return OpType::Load;
    if (value == "STORE") return OpType::Store;
    throw std::runtime_error("unsupported op_type: " + value);
}

struct Instruction {
    uint64_t id{};
    OpType type{OpType::Compute};
    uint32_t execute_latency{1};
    uint32_t memory_bytes{0};
    std::vector<uint64_t> deps{};
    Stage stage{Stage::Fetch};
    uint64_t enqueue_cycle{0};
    uint64_t issue_cycle{0};
    uint64_t complete_cycle{0};
};

}  // namespace accelsim
