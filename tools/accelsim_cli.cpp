#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "accelsim/core/simulator.hpp"
#include "accelsim/core/workload_runner.hpp"
#include "accelsim/model/config.hpp"

namespace fs = std::filesystem;
using namespace accelsim;

namespace {

void usage() {
    std::cerr
        << "usage:\n"
        << "  accelsim-lite run <workload.csv> [--compute-units N] [--memory-ports N] [--dispatch-queue N] [--ready-queue N]\n"
        << "  accelsim-lite compare <left.csv> <right.csv> [--compute-units N] [--memory-ports N] [--dispatch-queue N] [--ready-queue N]\n"
        << "  accelsim-lite benchmark all\n"
        << "  accelsim-lite report latest\n";
}

void apply_flag(SimulatorConfig& config, const std::string& key, const std::string& value) {
    const auto parsed = static_cast<uint32_t>(std::stoul(value));
    if (key == "--compute-units") config.compute_units = parsed;
    else if (key == "--memory-ports") config.memory_ports = parsed;
    else if (key == "--dispatch-queue") config.dispatch_queue_capacity = parsed;
    else if (key == "--ready-queue") config.ready_queue_capacity = parsed;
    else throw std::runtime_error("unknown flag: " + key);
}

void parse_optional_flags(SimulatorConfig& config, int argc, char** argv, int start_index) {
    for (int i = start_index; i < argc; i += 2) {
        if (i + 1 >= argc) {
            throw std::runtime_error("missing value for flag: " + std::string(argv[i]));
        }
        apply_flag(config, argv[i], argv[i + 1]);
    }
}

std::string benchmark_stamp() {
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", tm);
    return buffer;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            usage();
            return 1;
        }

        const std::string command = argv[1];

        if (command == "run") {
            if (argc < 3) {
                usage();
                return 1;
            }

            SimulatorConfig config;
            parse_optional_flags(config, argc, argv, 3);

            const std::string workload_path = argv[2];
            const auto workload = load_workload_csv(workload_path, &config);
            const auto result = run_workload(workload, config);

            write_report(result, "reports/latest");
            std::cout << summarize(result);
            return 0;
        }

        if (command == "compare") {
            if (argc < 4) {
                usage();
                return 1;
            }

            SimulatorConfig left_config;
            SimulatorConfig right_config;
            parse_optional_flags(left_config, argc, argv, 4);
            right_config = left_config;

            const auto left_workload = load_workload_csv(argv[2], &left_config);
            const auto right_workload = load_workload_csv(argv[3], &right_config);

            const auto left_result = run_workload(left_workload, left_config);
            const auto right_result = run_workload(right_workload, right_config);

            fs::create_directories("reports/latest");
            write_report(left_result, "reports/latest/left");
            write_report(right_result, "reports/latest/right");

            std::ofstream out("reports/latest/compare.txt");
            out << compare_runs(left_result, right_result);

            std::cout << compare_runs(left_result, right_result);
            return 0;
        }

        if (command == "benchmark" && argc >= 3 && std::string(argv[2]) == "all") {
            SimulatorConfig config;
            const std::vector<std::string> workloads = {
                "workloads/compute_heavy.csv",
                "workloads/memory_heavy.csv",
                "workloads/queue_pressure.csv",
                "workloads/mixed.csv"
            };

            const auto root = fs::path("reports/benchmarks") / benchmark_stamp();
            fs::create_directories(root);

            for (const auto& workload_path : workloads) {
                if (!fs::exists(workload_path)) continue;
                auto local_config = config;
                const auto workload = load_workload_csv(workload_path, &local_config);
                const auto result = run_workload(workload, local_config);
                const auto out_dir = root / local_config.workload_name;
                write_report(result, out_dir);
                std::cout << local_config.workload_name << " cycles=" << result.stats.total_cycles
                          << " throughput=" << result.stats.throughput()
                          << " top_bottleneck=" << result.stats.top_bottleneck() << "\n";
            }
            return 0;
        }

        if (command == "report" && argc >= 3 && std::string(argv[2]) == "latest") {
            std::ifstream in("reports/latest/summary.txt");
            if (!in) {
                throw std::runtime_error("reports/latest/summary.txt not found");
            }
            std::cout << in.rdbuf();
            return 0;
        }

        usage();
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
