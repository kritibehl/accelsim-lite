#pragma once
#include <unordered_set>
#include "accelsim/model/instruction.hpp"

namespace accelsim {

bool deps_satisfied(const Instruction& instruction, const std::unordered_set<uint64_t>& retired_ids);

}  // namespace accelsim
