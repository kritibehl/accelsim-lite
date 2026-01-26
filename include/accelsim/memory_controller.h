#pragma once
#include <cstdint>
#include <queue>
#include <vector>

namespace accelsim {

struct MemReq {
  uint64_t instr_id;
  int ready_cycle;
};

class MemoryController {
 public:
  explicit MemoryController(int fixed_latency) : fixed_latency_(fixed_latency) {}

  void issue(uint64_t instr_id, int now_cycle);
  std::vector<uint64_t> tick(int now_cycle);

 private:
  int fixed_latency_;
  std::queue<MemReq> q_;
};

} // namespace accelsim
