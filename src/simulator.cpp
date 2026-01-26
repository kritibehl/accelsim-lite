#include "accelsim/simulator.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace accelsim {

static std::vector<std::string> split_csv(const std::string& line) {
  std::vector<std::string> out;
  std::stringstream ss(line);
  std::string item;
  while (std::getline(ss, item, ',')) out.push_back(item);
  return out;
}

Simulator::Simulator(SimConfig cfg)
  : cfg_(cfg),
    dispatch_q_(cfg.dispatch_q_cap),
    mem_(cfg.mem_latency) {}

void Simulator::load_trace_csv(const std::string& path) {
  std::ifstream in(path);
  if (!in.is_open()) throw std::runtime_error("Failed to open trace csv");

  instrs_.clear();
  id_to_index_.clear();
  pc_ = 0;
  ready_list_.clear();

  std::string line;
  bool first = true;
  while (std::getline(in, line)) {
    if (first) { first = false; continue; } // header
    if (line.empty()) continue;
    auto cols = split_csv(line);
    if (cols.size() < 7) throw std::runtime_error("Bad CSV row: " + line);

    Instruction ins;
    ins.id = std::stoull(cols[0]);
    ins.op = parse_op(cols[1]);
    ins.src0 = std::stoi(cols[2]);
    ins.src1 = std::stoi(cols[3]);
    ins.dst = std::stoi(cols[4]);
    ins.is_mem = (std::stoi(cols[5]) != 0);
    ins.mem_bytes = std::stoi(cols[6]);
    ins.compute_latency = default_compute_latency(ins.op);
    ins.mem_latency = ins.is_mem ? cfg_.mem_latency : 0;

    id_to_index_[ins.id] = instrs_.size();
    instrs_.push_back(ins);
  }

  // sort by id to keep deterministic fetch order if input not sorted
  std::sort(instrs_.begin(), instrs_.end(),
            [](const Instruction& a, const Instruction& b){ return a.id < b.id; });

  id_to_index_.clear();
  for (size_t i = 0; i < instrs_.size(); i++) id_to_index_[instrs_[i].id] = i;

  pc_ = 0;
}

void Simulator::push_ready(uint64_t instr_id) {
  ready_list_.push_back(instr_id);
  std::sort(ready_list_.begin(), ready_list_.end()); // deterministic
}

int Simulator::run(const std::string& trace_out_path) {
  TraceWriter tw(trace_out_path);
  tw.write_header();

  int cycle = 0;
  int done_count = 0;
  const int total = (int)instrs_.size();

  while (done_count < total) {
    if (cycle > cfg_.max_cycles) throw std::runtime_error("Exceeded max_cycles safety limit");

    // 1) complete memory
    for (auto id : mem_.tick(cycle)) {
      auto& ins = instrs_.at(id_to_index_.at(id));
      // memory done -> ready for compute issue (or mark done if no compute)
      push_ready(id);
      tw.log(cycle, id, "MEM_DONE");
    }

    // 2) complete compute
    if (auto done = cu_.tick(cycle)) {
      auto id = *done;
      auto& ins = instrs_.at(id_to_index_.at(id));
      ins.state = InstrState::DONE;
      ins.done_cycle = cycle;
      done_count++;
      tw.log(cycle, id, "DONE");
    }

    // 3) issue from ready list (single-issue)
    if (!cu_.busy() && !ready_list_.empty()) {
      uint64_t id = ready_list_.front();
      ready_list_.erase(ready_list_.begin());
      auto& ins = instrs_.at(id_to_index_.at(id));

      ins.state = InstrState::EXECUTING;
      ins.issue_cycle = (ins.issue_cycle == -1) ? cycle : ins.issue_cycle;
      cu_.issue(id, cycle, ins.compute_latency);
      tw.log(cycle, id, "ISSUE_COMPUTE");
    }

    // 4) dispatch -> memory or ready
    if (!dispatch_q_.empty()) {
      uint64_t id = dispatch_q_.front();
      auto& ins = instrs_.at(id_to_index_.at(id));

      // dispatch only if instruction is in DISPATCHED state
      if (ins.state == InstrState::DISPATCHED) {
        (void)dispatch_q_.pop();
        if (ins.is_mem) {
          mem_.issue(id, cycle);
          ins.state = InstrState::ISSUED;
          tw.log(cycle, id, "ISSUE_MEM");
        } else {
          ins.state = InstrState::ISSUED;
          push_ready(id);
          tw.log(cycle, id, "READY");
        }
      }
    }

    // 5) fetch/decode/dispatch next instruction
    if (pc_ < instrs_.size() && !dispatch_q_.full()) {
      auto& ins = instrs_[pc_];
      if (ins.state == InstrState::NEW) {
        ins.state = InstrState::FETCHED;
        ins.fetch_cycle = cycle;
        tw.log(cycle, ins.id, "FETCH");
      }
      // decode same cycle for MVP
      ins.state = InstrState::DECODED;
      tw.log(cycle, ins.id, "DECODE");

      ins.state = InstrState::DISPATCHED;
      dispatch_q_.push(ins.id);
      tw.log(cycle, ins.id, "DISPATCH");
      pc_++;
    }

    cycle++;
  }

  return cycle;
}

} // namespace accelsim
