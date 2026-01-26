#include "accelsim/simulator.h"
#include <gtest/gtest.h>
#include <fstream>
#include <sstream>

#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR "."
#endif

static std::string slurp(const std::string& path) {
  std::ifstream in(path);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

TEST(Simulator, DeterministicOutput) {
  accelsim::SimConfig cfg;
  accelsim::Simulator sim1(cfg), sim2(cfg);

  std::string trace = std::string(PROJECT_SOURCE_DIR) + "/traces/sample_trace.csv";
  sim1.load_trace_csv(trace);
  sim2.load_trace_csv(trace);

  sim1.run("out1.csv");
  sim2.run("out2.csv");

  EXPECT_EQ(slurp("out1.csv"), slurp("out2.csv"));
}
