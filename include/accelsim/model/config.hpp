#pragma once
#include <cstdint>
#include <string>

namespace accelsim {

struct SimulatorConfig {
    uint32_t fetch_width{4};
    uint32_t decode_width{4};
    uint32_t dispatch_width{4};
    uint32_t issue_width{2};

    uint32_t compute_units{2};
    uint32_t memory_ports{1};

    uint32_t dispatch_queue_capacity{16};
    uint32_t ready_queue_capacity{16};

    std::string workload_name{"unknown"};
};

}  // namespace accelsim
