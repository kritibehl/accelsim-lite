#include "accelsim/simulator.h"
#include <iostream>

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: accelsim_cli <input_trace.csv> <output_trace.csv>\n";
    return 1;
  }

  accelsim::SimConfig cfg;
  accelsim::Simulator sim(cfg);

  sim.load_trace_csv(argv[1]);
  int cycles = sim.run(argv[2]);

  std::cout << "Done in " << cycles << " cycles\n";
  return 0;
}
