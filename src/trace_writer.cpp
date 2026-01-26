#include "accelsim/trace_writer.h"
#include <stdexcept>

namespace accelsim {

TraceWriter::TraceWriter(const std::string& path) : out_(path) {
  if (!out_.is_open()) throw std::runtime_error("Failed to open trace output file");
}

void TraceWriter::write_header() {
  out_ << "cycle,instr_id,event\n";
}

void TraceWriter::log(int cycle, uint64_t instr_id, const std::string& event) {
  out_ << cycle << "," << instr_id << "," << event << "\n";
}

} // namespace accelsim
