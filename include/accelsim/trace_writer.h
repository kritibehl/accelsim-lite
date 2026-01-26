#pragma once
#include <fstream>
#include <string>

namespace accelsim {

class TraceWriter {
 public:
  explicit TraceWriter(const std::string& path);
  void write_header();
  void log(int cycle, uint64_t instr_id, const std::string& event);
 private:
  std::ofstream out_;
};

} // namespace accelsim
