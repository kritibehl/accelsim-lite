#pragma once
#include <cstdint>
#include <string>

namespace accelsim {

enum class Op {
  ADD,
  MATMUL,
  LOAD,
  STORE,
  UNKNOWN
};

enum class InstrState {
  NEW,
  FETCHED,
  DECODED,
  DISPATCHED,
  ISSUED,
  EXECUTING,
  DONE
};

struct Instruction {
  uint64_t id{};
  Op op{Op::UNKNOWN};

  int src0{0};
  int src1{0};
  int dst{0};

  bool is_mem{false};
  int mem_bytes{0};

  InstrState state{InstrState::NEW};

  // Timing (cycle numbers). -1 means unset.
  int fetch_cycle{-1};
  int issue_cycle{-1};
  int done_cycle{-1};

  int compute_latency{1};  // filled from op
  int mem_latency{0};      // for memory ops, if used
};

Op parse_op(const std::string& s);
int default_compute_latency(Op op);

} // namespace accelsim
