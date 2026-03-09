#include "accelsim/core/simulator.hpp"
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include "accelsim/core/pipeline.hpp"
#include "accelsim/core/scheduler.hpp"

namespace accelsim {
namespace {

struct ExecSlot {
    std::size_t index{};
    uint32_t remaining{};
    bool uses_memory{};
};

void bump(std::map<std::string, uint64_t>& values, const std::string& key, uint64_t amount = 1) {
    values[key] += amount;
}

void update_max(std::map<std::string, uint64_t>& values, const std::string& key, uint64_t candidate) {
    auto it = values.find(key);
    if (it == values.end() || candidate > it->second) {
        values[key] = candidate;
    }
}

bool has_work_left(std::size_t next_to_fetch,
                   std::size_t total,
                   const std::deque<std::size_t>& fetch_q,
                   const std::deque<std::size_t>& decode_q,
                   const std::deque<std::size_t>& dispatch_q,
                   const std::deque<std::size_t>& ready_q,
                   const std::deque<std::size_t>& retire_q,
                   const std::vector<ExecSlot>& exec_slots) {
    return next_to_fetch < total || !fetch_q.empty() || !decode_q.empty() || !dispatch_q.empty() ||
           !ready_q.empty() || !retire_q.empty() || !exec_slots.empty();
}

void write_map_csv(const std::filesystem::path& path, const std::string& header_a,
                   const std::string& header_b, const std::map<std::string, uint64_t>& values) {
    std::ofstream out(path);
    out << header_a << "," << header_b << "\n";
    for (const auto& [key, value] : values) {
        out << key << "," << value << "\n";
    }
}

std::string format_double(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(4) << value;
    return out.str();
}

std::string map_to_json(const std::map<std::string, uint64_t>& values) {
    std::ostringstream out;
    out << "{";
    bool first = true;
    for (const auto& [key, value] : values) {
        if (!first) out << ",";
        first = false;
        out << "\"" << key << "\":" << value;
    }
    out << "}";
    return out.str();
}

}  // namespace

RunResult run_workload(const std::vector<Instruction>& workload, const SimulatorConfig& config) {
    RunResult result;
    result.config = config;
    result.instructions = workload;

    auto& stats = result.stats;
    bump(stats.stage_busy_cycles, "Fetch", 0);
    bump(stats.stage_busy_cycles, "Decode", 0);
    bump(stats.stage_busy_cycles, "Dispatch", 0);
    bump(stats.stage_busy_cycles, "Issue", 0);
    bump(stats.stage_busy_cycles, "Execute", 0);
    bump(stats.stage_busy_cycles, "Retire", 0);

    bump(stats.stall_counts, "DispatchQueueFull", 0);
    bump(stats.stall_counts, "ReadyQueueFull", 0);
    bump(stats.stall_counts, "WaitingDependency", 0);
    bump(stats.stall_counts, "NoComputeUnit", 0);
    bump(stats.stall_counts, "NoMemoryPort", 0);

    std::deque<std::size_t> fetch_q;
    std::deque<std::size_t> decode_q;
    std::deque<std::size_t> dispatch_q;
    std::deque<std::size_t> ready_q;
    std::deque<std::size_t> retire_q;
    std::vector<ExecSlot> exec_slots;
    std::unordered_set<uint64_t> retired_ids;

    std::size_t next_to_fetch = 0;
    uint64_t cycle = 0;
    uint32_t compute_in_use = 0;
    uint32_t memory_in_use = 0;

    while (has_work_left(next_to_fetch, result.instructions.size(), fetch_q, decode_q, dispatch_q,
                         ready_q, retire_q, exec_slots)) {
        while (!retire_q.empty()) {
            const auto idx = retire_q.front();
            retire_q.pop_front();
            auto& instruction = result.instructions[idx];
            instruction.stage = Stage::Done;
            retired_ids.insert(instruction.id);
            stats.completed_ops += 1;
            stats.total_latency += (instruction.complete_cycle - instruction.enqueue_cycle);
        }

        std::vector<ExecSlot> remaining_slots;
        std::vector<std::size_t> finished;
        for (auto slot : exec_slots) {
            if (slot.remaining > 0) {
                slot.remaining -= 1;
            }
            if (slot.remaining == 0) {
                finished.push_back(slot.index);
                if (slot.uses_memory) {
                    if (memory_in_use == 0) throw std::runtime_error("memory resource underflow");
                    memory_in_use -= 1;
                } else {
                    if (compute_in_use == 0) throw std::runtime_error("compute resource underflow");
                    compute_in_use -= 1;
                }
            } else {
                remaining_slots.push_back(slot);
            }
        }
        exec_slots = remaining_slots;
        for (const auto idx : finished) {
            auto& instruction = result.instructions[idx];
            instruction.complete_cycle = cycle;
            instruction.stage = Stage::Retire;
            retire_q.push_back(idx);
        }

        const auto dispatch_span = dispatch_q.size();
        for (std::size_t i = 0; i < dispatch_span; ++i) {
            const auto idx = dispatch_q.front();
            dispatch_q.pop_front();
            auto& instruction = result.instructions[idx];
            if (deps_satisfied(instruction, retired_ids)) {
                if (ready_q.size() < config.ready_queue_capacity) {
                    instruction.stage = Stage::Ready;
                    ready_q.push_back(idx);
                } else {
                    bump(stats.stall_counts, "ReadyQueueFull");
                    dispatch_q.push_back(idx);
                }
            } else {
                bump(stats.stall_counts, "WaitingDependency");
                dispatch_q.push_back(idx);
            }
        }

        uint32_t issued = 0;
        const auto ready_span = ready_q.size();
        for (std::size_t i = 0; i < ready_span; ++i) {
            const auto idx = ready_q.front();
            ready_q.pop_front();
            auto& instruction = result.instructions[idx];

            if (issued >= config.issue_width) {
                ready_q.push_back(idx);
                continue;
            }

            const bool uses_memory = is_memory_op(instruction.type);
            if (uses_memory) {
                if (memory_in_use >= config.memory_ports) {
                    bump(stats.stall_counts, "NoMemoryPort");
                    ready_q.push_back(idx);
                    continue;
                }
                memory_in_use += 1;
            } else {
                if (compute_in_use >= config.compute_units) {
                    bump(stats.stall_counts, "NoComputeUnit");
                    ready_q.push_back(idx);
                    continue;
                }
                compute_in_use += 1;
            }

            issued += 1;
            instruction.stage = Stage::Execute;
            instruction.issue_cycle = cycle;
            exec_slots.push_back(ExecSlot{idx, instruction.execute_latency, uses_memory});
        }

        uint32_t dispatched = 0;
        while (!decode_q.empty() && dispatched < config.dispatch_width) {
            if (dispatch_q.size() >= config.dispatch_queue_capacity) {
                bump(stats.stall_counts, "DispatchQueueFull");
                break;
            }
            const auto idx = decode_q.front();
            decode_q.pop_front();
            result.instructions[idx].stage = Stage::Dispatch;
            dispatch_q.push_back(idx);
            dispatched += 1;
        }

        uint32_t decoded = 0;
        while (!fetch_q.empty() && decoded < config.decode_width) {
            const auto idx = fetch_q.front();
            fetch_q.pop_front();
            result.instructions[idx].stage = Stage::Decode;
            decode_q.push_back(idx);
            decoded += 1;
        }

        uint32_t fetched = 0;
        while (next_to_fetch < result.instructions.size() && fetched < config.fetch_width) {
            auto& instruction = result.instructions[next_to_fetch];
            instruction.stage = Stage::Fetch;
            instruction.enqueue_cycle = cycle;
            fetch_q.push_back(next_to_fetch);
            next_to_fetch += 1;
            fetched += 1;
        }

        if (next_to_fetch < result.instructions.size()) bump(stats.stage_busy_cycles, "Fetch");
        if (!fetch_q.empty()) bump(stats.stage_busy_cycles, "Decode");
        if (!decode_q.empty() || !dispatch_q.empty()) bump(stats.stage_busy_cycles, "Dispatch");
        if (!ready_q.empty()) bump(stats.stage_busy_cycles, "Issue");
        if (!exec_slots.empty()) bump(stats.stage_busy_cycles, "Execute");
        if (!retire_q.empty()) bump(stats.stage_busy_cycles, "Retire");

        update_max(stats.max_queue_occupancy, "fetch_q", fetch_q.size());
        update_max(stats.max_queue_occupancy, "decode_q", decode_q.size());
        update_max(stats.max_queue_occupancy, "dispatch_q", dispatch_q.size());
        update_max(stats.max_queue_occupancy, "ready_q", ready_q.size());
        update_max(stats.max_queue_occupancy, "execute_slots", exec_slots.size());
        update_max(stats.max_queue_occupancy, "retire_q", retire_q.size());

        cycle += 1;
    }

    result.stats.total_cycles = cycle;
    return result;
}

std::string summarize(const RunResult& result) {
    std::ostringstream out;
    out << "workload=" << result.config.workload_name << "\n";
    out << "total_cycles=" << result.stats.total_cycles << "\n";
    out << "completed_ops=" << result.stats.completed_ops << "\n";
    out << "throughput=" << format_double(result.stats.throughput()) << "\n";
    out << "average_latency=" << format_double(result.stats.average_latency()) << "\n";
    out << "top_bottleneck=" << result.stats.top_bottleneck() << "\n";
    out << "\n[stage_busy_cycles]\n";
    for (const auto& [name, value] : result.stats.stage_busy_cycles) {
        out << name << "=" << value << "\n";
    }
    out << "\n[stall_counts]\n";
    for (const auto& [name, value] : result.stats.stall_counts) {
        out << name << "=" << value << "\n";
    }
    out << "\n[max_queue_occupancy]\n";
    for (const auto& [name, value] : result.stats.max_queue_occupancy) {
        out << name << "=" << value << "\n";
    }
    return out.str();
}

std::string compare_runs(const RunResult& left, const RunResult& right) {
    auto delta_i64 = [](uint64_t a, uint64_t b) -> long long {
        return static_cast<long long>(b) - static_cast<long long>(a);
    };

    std::ostringstream out;
    out << "left=" << left.config.workload_name << "\n";
    out << "right=" << right.config.workload_name << "\n";
    out << "cycles_delta=" << delta_i64(left.stats.total_cycles, right.stats.total_cycles) << "\n";
    out << "throughput_delta="
        << format_double(right.stats.throughput() - left.stats.throughput()) << "\n";
    out << "avg_latency_delta="
        << format_double(right.stats.average_latency() - left.stats.average_latency()) << "\n";
    out << "left_top_bottleneck=" << left.stats.top_bottleneck() << "\n";
    out << "right_top_bottleneck=" << right.stats.top_bottleneck() << "\n";
    return out.str();
}

void write_report(const RunResult& result, const std::filesystem::path& out_dir) {
    std::filesystem::create_directories(out_dir);

    {
        std::ofstream out(out_dir / "summary.txt");
        out << summarize(result);
    }

    {
        std::ofstream out(out_dir / "summary.json");
        out << "{\n";
        out << "  \"workload\":\"" << result.config.workload_name << "\",\n";
        out << "  \"total_cycles\":" << result.stats.total_cycles << ",\n";
        out << "  \"completed_ops\":" << result.stats.completed_ops << ",\n";
        out << "  \"throughput\":" << format_double(result.stats.throughput()) << ",\n";
        out << "  \"average_latency\":" << format_double(result.stats.average_latency()) << ",\n";
        out << "  \"top_bottleneck\":\"" << result.stats.top_bottleneck() << "\",\n";
        out << "  \"stage_busy_cycles\":" << map_to_json(result.stats.stage_busy_cycles) << ",\n";
        out << "  \"stall_counts\":" << map_to_json(result.stats.stall_counts) << ",\n";
        out << "  \"max_queue_occupancy\":" << map_to_json(result.stats.max_queue_occupancy) << "\n";
        out << "}\n";
    }

    write_map_csv(out_dir / "utilization.csv", "stage", "busy_cycles", result.stats.stage_busy_cycles);
    write_map_csv(out_dir / "stalls.csv", "stall_reason", "count", result.stats.stall_counts);
    write_map_csv(out_dir / "queue_occupancy.csv", "queue", "max_occupancy",
                  result.stats.max_queue_occupancy);

    {
        std::ofstream out(out_dir / "completed_trace.csv");
        out << "id,op_type,enqueue_cycle,issue_cycle,complete_cycle\n";
        for (const auto& instruction : result.instructions) {
            out << instruction.id << ","
                << to_string(instruction.type) << ","
                << instruction.enqueue_cycle << ","
                << instruction.issue_cycle << ","
                << instruction.complete_cycle << "\n";
        }
    }
}

}  // namespace accelsim
