#pragma once
#include "accelsim/model/instruction.hpp"

namespace accelsim {

inline bool is_memory_op(OpType type) {
    return type == OpType::Load || type == OpType::Store;
}

}  // namespace accelsim
