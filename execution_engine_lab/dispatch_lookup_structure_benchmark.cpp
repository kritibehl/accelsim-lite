#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using Clock = std::chrono::high_resolution_clock;

struct Result {
    std::string workload;
    std::string structure;
    int entries;
    int lookups;
    int hits;
    int misses;
    double elapsed_ms;
};

static std::vector<std::string> make_keys(int count, const std::string& prefix) {
    std::vector<std::string> out;
    out.reserve(count);

    for (int i = 0; i < count; ++i) {
        out.push_back(prefix + "_" + std::to_string(i));
    }

    return out;
}

template <typename MapType>
static Result run_lookup_benchmark(
    const std::string& workload,
    const std::string& structure_name,
    const std::vector<std::string>& inserted_keys,
    const std::vector<std::string>& lookup_keys)
{
    MapType table;

    for (size_t i = 0; i < inserted_keys.size(); ++i) {
        table[inserted_keys[i]] = static_cast<int>(i);
    }

    int hits = 0;
    int misses = 0;

    auto start = Clock::now();

    for (const auto& k : lookup_keys) {
        auto it = table.find(k);

        if (it != table.end()) {
            ++hits;
        } else {
            ++misses;
        }
    }

    auto end = Clock::now();

    double elapsed_ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    return {
        workload,
        structure_name,
        static_cast<int>(inserted_keys.size()),
        static_cast<int>(lookup_keys.size()),
        hits,
        misses,
        elapsed_ms
    };
}

int main() {
    std::vector<Result> results;

    struct WorkloadSpec {
        std::string name;
        int entry_count;
        int lookup_count;
        double miss_ratio;
    };

    std::vector<WorkloadSpec> workloads = {
        {"opcode_dispatch_small", 16, 100000, 0.05},
        {"opcode_dispatch_medium", 128, 200000, 0.10},
        {"shape_metadata_diverse", 512, 300000, 0.20},
        {"inline_cache_state_diverse", 1024, 400000, 0.30}
    };

    std::mt19937 rng(42);

    for (const auto& w : workloads) {
        auto inserted = make_keys(w.entry_count, w.name);

        std::vector<std::string> lookups;
        lookups.reserve(w.lookup_count);

        std::uniform_int_distribution<int> hit_dist(0, w.entry_count - 1);
        std::uniform_real_distribution<double> miss_flip(0.0, 1.0);

        for (int i = 0; i < w.lookup_count; ++i) {
            if (miss_flip(rng) < w.miss_ratio) {
                lookups.push_back("missing_key_" + std::to_string(i));
            } else {
                lookups.push_back(inserted[hit_dist(rng)]);
            }
        }

        results.push_back(
            run_lookup_benchmark<std::unordered_map<std::string, int>>(
                w.name,
                "unordered_map_hash_table",
                inserted,
                lookups
            )
        );

        results.push_back(
            run_lookup_benchmark<std::map<std::string, int>>(
                w.name,
                "map_tree_index",
                inserted,
                lookups
            )
        );
    }

    std::ofstream json_out(
        "execution_engine_lab/lookup_structure_benchmark_summary.json");

    json_out << "{\n";
    json_out << "  \"scope\": "
             << "\"educational runtime metadata lookup benchmark; "
             << "not a V8 benchmark or production engine benchmark\",\n";
    json_out << "  \"results\": [\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];

        json_out << "    {\n";
        json_out << "      \"workload\": \"" << r.workload << "\",\n";
        json_out << "      \"structure\": \"" << r.structure << "\",\n";
        json_out << "      \"entries\": " << r.entries << ",\n";
        json_out << "      \"lookups\": " << r.lookups << ",\n";
        json_out << "      \"hits\": " << r.hits << ",\n";
        json_out << "      \"misses\": " << r.misses << ",\n";
        json_out << "      \"elapsed_ms\": "
                 << std::fixed << std::setprecision(4)
                 << r.elapsed_ms << "\n";
        json_out << "    }";

        if (i + 1 != results.size()) {
            json_out << ",";
        }

        json_out << "\n";
    }

    json_out << "  ]\n";
    json_out << "}\n";

    std::ofstream report(
        "execution_engine_lab/lookup_structure_benchmark_report.md");

    report << "# Runtime Lookup Structure Benchmark\n\n";
    report << "> Scope: educational data-structure tradeoff experiment "
              "for runtime metadata lookup. "
              "Not a V8 benchmark or production engine benchmark.\n\n";

    report << "| Workload | Structure | Entries | Lookups | Hits | Misses | Elapsed ms |\n";
    report << "|---|---|---|---|---|---|---|\n";

    for (const auto& r : results) {
        report << "| "
               << r.workload << " | "
               << r.structure << " | "
               << r.entries << " | "
               << r.lookups << " | "
               << r.hits << " | "
               << r.misses << " | "
               << std::fixed << std::setprecision(4)
               << r.elapsed_ms << " |\n";
    }

    report << "\n## Tradeoff notes\n\n";
    report << "- unordered_map models average-case hash-table lookup behavior\n";
    report << "- map models ordered tree-backed lookup behavior\n";
    report << "- shape-diverse workloads increase metadata lookup pressure\n";
    report << "- runtime metadata access patterns can affect lookup efficiency\n";

    std::cout << "Generated execution_engine_lab/lookup_structure_benchmark_summary.json\n";
    std::cout << "Generated execution_engine_lab/lookup_structure_benchmark_report.md\n";

    return 0;
}
