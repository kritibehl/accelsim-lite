#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

struct Task {
    int id;
    int compute_cost;
    int transfer_cost;
};

struct WorkerStats {
    int worker_id = 0;
    int tasks_completed = 0;
    int compute_cycles = 0;
    int transfer_cycles = 0;
    int simulated_cycles = 0;
};

struct ScenarioResult {
    std::string scenario;
    int worker_threads = 0;
    int task_count = 0;
    int total_compute_cycles = 0;
    int total_transfer_cycles = 0;
    int total_worker_cycles = 0;
    int max_worker_cycles = 0;
    int min_worker_cycles = 0;
    int imbalance_cycles = 0;
    double throughput_tasks_per_cycle = 0.0;
    std::string bottleneck;
    std::vector<WorkerStats> workers;
};

std::vector<Task> make_balanced_workload(int task_count) {
    std::vector<Task> tasks;
    tasks.reserve(task_count);

    for (int i = 0; i < task_count; ++i) {
        tasks.push_back(Task{i, 8 + (i % 3), 3 + (i % 2)});
    }

    return tasks;
}

std::vector<Task> make_imbalanced_workload(int task_count) {
    std::vector<Task> tasks;
    tasks.reserve(task_count);

    for (int i = 0; i < task_count; ++i) {
        int heavy = (i % 7 == 0) ? 18 : 6;
        int transfer = (i % 5 == 0) ? 10 : 3;
        tasks.push_back(Task{i, heavy, transfer});
    }

    return tasks;
}

std::string classify_bottleneck(const ScenarioResult& result) {
    const double imbalance_ratio =
        result.max_worker_cycles == 0
            ? 0.0
            : static_cast<double>(result.imbalance_cycles) /
                  static_cast<double>(result.max_worker_cycles);

    const double transfer_ratio =
        result.total_worker_cycles == 0
            ? 0.0
            : static_cast<double>(result.total_transfer_cycles) /
                  static_cast<double>(result.total_worker_cycles);

    if (result.worker_threads == 1) {
        return "serial-bound";
    }

    if (imbalance_ratio > 0.35) {
        return "worker-imbalance-limited";
    }

    if (transfer_ratio > 0.32) {
        return "contention-limited";
    }

    return "parallelism-improved";
}

ScenarioResult run_scenario(const std::string& scenario,
                            const std::vector<Task>& tasks,
                            int worker_threads) {
    std::mutex queue_mu;
    std::size_t next_index = 0;

    std::vector<WorkerStats> stats(worker_threads);
    std::vector<std::thread> workers;

    for (int worker_id = 0; worker_id < worker_threads; ++worker_id) {
        stats[worker_id].worker_id = worker_id;

        workers.emplace_back([&, worker_id]() {
            while (true) {
                Task task;

                {
                    std::lock_guard<std::mutex> lock(queue_mu);
                    if (next_index >= tasks.size()) {
                        break;
                    }

                    task = tasks[next_index++];
                }

                // Deterministic simulated work accounting.
                stats[worker_id].tasks_completed += 1;
                stats[worker_id].compute_cycles += task.compute_cost;
                stats[worker_id].transfer_cycles += task.transfer_cost;
                stats[worker_id].simulated_cycles +=
                    task.compute_cost + task.transfer_cost;
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    ScenarioResult result;
    result.scenario = scenario;
    result.worker_threads = worker_threads;
    result.task_count = static_cast<int>(tasks.size());
    result.workers = stats;

    for (const auto& worker : stats) {
        result.total_compute_cycles += worker.compute_cycles;
        result.total_transfer_cycles += worker.transfer_cycles;
        result.total_worker_cycles += worker.simulated_cycles;
    }

    std::vector<int> worker_cycles;
    for (const auto& worker : stats) {
        worker_cycles.push_back(worker.simulated_cycles);
    }

    result.max_worker_cycles = *std::max_element(worker_cycles.begin(), worker_cycles.end());
    result.min_worker_cycles = *std::min_element(worker_cycles.begin(), worker_cycles.end());
    result.imbalance_cycles = result.max_worker_cycles - result.min_worker_cycles;

    result.throughput_tasks_per_cycle =
        result.max_worker_cycles == 0
            ? 0.0
            : static_cast<double>(result.task_count) /
                  static_cast<double>(result.max_worker_cycles);

    result.bottleneck = classify_bottleneck(result);
    return result;
}

std::string json_escape(const std::string& value) {
    std::ostringstream out;
    for (char c : value) {
        if (c == '"') {
            out << "\\\"";
        } else {
            out << c;
        }
    }
    return out.str();
}

void write_json(const std::vector<ScenarioResult>& results,
                const std::string& path) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"std::thread workload scaling experiment; simulated cycle accounting, not OS scheduler benchmarking\",\n";
    out << "  \"results\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];

        out << "    {\n";
        out << "      \"scenario\": \"" << json_escape(r.scenario) << "\",\n";
        out << "      \"worker_threads\": " << r.worker_threads << ",\n";
        out << "      \"task_count\": " << r.task_count << ",\n";
        out << "      \"total_compute_cycles\": " << r.total_compute_cycles << ",\n";
        out << "      \"total_transfer_cycles\": " << r.total_transfer_cycles << ",\n";
        out << "      \"max_worker_cycles\": " << r.max_worker_cycles << ",\n";
        out << "      \"min_worker_cycles\": " << r.min_worker_cycles << ",\n";
        out << "      \"imbalance_cycles\": " << r.imbalance_cycles << ",\n";
        out << "      \"throughput_tasks_per_cycle\": "
            << std::fixed << std::setprecision(6)
            << r.throughput_tasks_per_cycle << ",\n";
        out << "      \"bottleneck_classification\": \"" << r.bottleneck << "\",\n";
        out << "      \"workers\": [\n";

        for (std::size_t j = 0; j < r.workers.size(); ++j) {
            const auto& w = r.workers[j];

            out << "        {\n";
            out << "          \"worker_id\": " << w.worker_id << ",\n";
            out << "          \"tasks_completed\": " << w.tasks_completed << ",\n";
            out << "          \"compute_cycles\": " << w.compute_cycles << ",\n";
            out << "          \"transfer_cycles\": " << w.transfer_cycles << ",\n";
            out << "          \"simulated_cycles\": " << w.simulated_cycles << "\n";
            out << "        }";

            if (j + 1 != r.workers.size()) {
                out << ",";
            }

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (i + 1 != results.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
}

void write_markdown(const std::vector<ScenarioResult>& results,
                    const std::string& path) {
    std::ofstream out(path);

    out << "# Threaded Workload Report\n\n";
    out << "> Scope: std::thread workload scaling experiment with deterministic simulated-cycle accounting. This is not a hardware timing benchmark.\n\n";

    out << "| Scenario | Workers | Tasks | Max Worker Cycles | Imbalance | Throughput | Bottleneck |\n";
    out << "|---|---:|---:|---:|---:|---:|---|\n";

    for (const auto& r : results) {
        out << "| " << r.scenario
            << " | " << r.worker_threads
            << " | " << r.task_count
            << " | " << r.max_worker_cycles
            << " | " << r.imbalance_cycles
            << " | " << std::fixed << std::setprecision(6)
            << r.throughput_tasks_per_cycle
            << " | " << r.bottleneck << " |\n";
    }

    out << "\n## Key observations\n\n";
    out << "- Worker scaling improves throughput when work is balanced and contention is low.\n";
    out << "- Imbalanced partitioning can limit parallel speedup even when more worker threads are available.\n";
    out << "- Transfer-heavy tasks increase contention pressure and reduce effective parallel scaling.\n";
    out << "- Bottleneck classifications are workload-level labels, not hardware performance claims.\n";

    out << "\n## Safe interpretation\n\n";
    out << "This experiment demonstrates C++ threading, shared-queue coordination, worker-level accounting, and bottleneck classification for synthetic accelerator-style workloads.\n";
}

int main() {
    const int task_count = 48;
    const std::vector<int> worker_sweep = {1, 2, 4};

    const auto balanced = make_balanced_workload(task_count);
    const auto imbalanced = make_imbalanced_workload(task_count);

    std::vector<ScenarioResult> results;

    for (int workers : worker_sweep) {
        results.push_back(run_scenario("balanced_partition", balanced, workers));
        results.push_back(run_scenario("imbalanced_partition", imbalanced, workers));
    }

    write_json(results, "parallel_execution/threaded_worker_sweep.json");
    write_markdown(results, "parallel_execution/threaded_workload_report.md");

    std::cout << "Generated parallel_execution/threaded_worker_sweep.json\n";
    std::cout << "Generated parallel_execution/threaded_workload_report.md\n";

    return 0;
}
