#pragma once
#include "accelsim/instruction.h"
#include "accelsim/queue.h"
#include "accelsim/memory_controller.h"
#include "accelsim/compute_unit.h"
#include "accelsim/trace_writer.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace accelsim {

struct SimConfig {
  int dispatch_q_cap{8};
  int mem_latency{20};
  int max_cycles{200000}; // safety
};

class Simulator {
 public:
  explicit Simulator(SimConfig cfg);

  void load_trace_csv(const std::string& path);
  int run(const std::string& trace_out_path);

  const std::vector<Instruction>& instructions() const { return instrs_; }

 private:
  SimConfig cfg_;
  std::vector<Instruction> instrs_;
  std::unordered_map<uint64_t, size_t> id_to_index_;

  // pipeline storage
  size_t pc_{0};
  BoundedQueue<uint64_t> dispatch_q_;
  std::vector<uint64_t> ready_list_; // deterministic: keep sorted by id
  MemoryController mem_;
  ComputeUnit cu_;

  void push_ready(uint64_t instr_id);
};

} // namespace accelsim
