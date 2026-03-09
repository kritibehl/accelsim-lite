#include "accelsim/core/scheduler.hpp"

namespace accelsim {

bool deps_satisfied(const Instruction& instruction, const std::unordered_set<uint64_t>& retired_ids) {
    for (uint64_t dep : instruction.deps) {
        if (retired_ids.find(dep) == retired_ids.end()) {
            return false;
        }
    }
    return true;
}

}  // namespace accelsim
