#include "accelsim/memory_controller.h"

namespace accelsim {

void MemoryController::issue(uint64_t instr_id, int now_cycle) {
  q_.push(MemReq{instr_id, now_cycle + fixed_latency_});
}

std::vector<uint64_t> MemoryController::tick(int now_cycle) {
  std::vector<uint64_t> done;
  while (!q_.empty() && q_.front().ready_cycle <= now_cycle) {
    done.push_back(q_.front().instr_id);
    q_.pop();
  }
  return done;
}

} // namespace accelsim
