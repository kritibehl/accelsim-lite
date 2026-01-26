#include "accelsim/instruction.h"
#include <algorithm>

namespace accelsim {

Op parse_op(const std::string& s) {
  std::string t = s;
  std::transform(t.begin(), t.end(), t.begin(), ::toupper);
  if (t == "ADD") return Op::ADD;
  if (t == "MATMUL") return Op::MATMUL;
  if (t == "LOAD") return Op::LOAD;
  if (t == "STORE") return Op::STORE;
  return Op::UNKNOWN;
}

int default_compute_latency(Op op) {
  switch (op) {
    case Op::ADD: return 1;
    case Op::MATMUL: return 6;   // simplified
    case Op::LOAD: return 1;     // compute part minimal; memory handled elsewhere
    case Op::STORE: return 1;
    default: return 1;
  }
}

} // namespace accelsim
