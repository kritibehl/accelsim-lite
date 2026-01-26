#pragma once
#include <cstdint>
#include <optional>

namespace accelsim {

class ComputeUnit {
 public:
  bool busy() const { return current_.has_value(); }
  std::optional<uint64_t> current() const { return current_; }

  void issue(uint64_t instr_id, int now_cycle, int latency);
  std::optional<uint64_t> tick(int now_cycle);

 private:
  std::optional<uint64_t> current_;
  int done_cycle_{-1};
};

} // namespace accelsim
