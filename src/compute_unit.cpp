#include "accelsim/compute_unit.h"

namespace accelsim {

void ComputeUnit::issue(uint64_t instr_id, int now_cycle, int latency) {
  current_ = instr_id;
  done_cycle_ = now_cycle + latency;
}

std::optional<uint64_t> ComputeUnit::tick(int now_cycle) {
  if (!current_.has_value()) return std::nullopt;
  if (now_cycle >= done_cycle_) {
    auto done = current_;
    current_.reset();
    done_cycle_ = -1;
    return done;
  }
  return std::nullopt;
}

} // namespace accelsim
