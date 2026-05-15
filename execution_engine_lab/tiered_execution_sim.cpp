#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Invocation {
    std::string trace_name;
    std::string operand_type;
    std::string branch_direction;
    int opcode_count;
    int basic_block_id;
};

struct TraceProfile {
    std::string trace_name;
    int invocation_count = 0;
    int opcode_count_total = 0;
    int repeated_branch_path_count = 0;
    int basic_block_reuse_count = 0;
    bool hot_trace = false;
    std::string dominant_branch_direction;
    int dominant_basic_block = -1;
};

struct GuardCase {
    std::string trace_name;
    int invocation_index = 0;
    bool optimized_path_attempted = false;
    bool optimized_path_taken = false;
    bool guard_failed = false;
    std::string guard_failure_reason;
    std::string execution_tier;
    std::string fallback_action;
};

struct TieringSummary {
    std::string trace_name;
    int total_invocations = 0;
    int hot_threshold = 3;
    int optimized_attempts = 0;
    int optimized_hits = 0;
    int deopt_count = 0;
    double optimized_hit_rate = 0.0;
    std::string final_status;
};

std::vector<Invocation> build_invocations() {
    return {
        {"arith_trace", "numeric", "none", 8, 1},
        {"arith_trace", "numeric", "none", 8, 1},
        {"arith_trace", "numeric", "none", 8, 1},
        {"arith_trace", "numeric", "none", 8, 1},
        {"arith_trace", "string_like", "none", 8, 1},

        {"branch_trace", "numeric", "taken", 6, 2},
        {"branch_trace", "numeric", "taken", 6, 2},
        {"branch_trace", "numeric", "taken", 6, 2},
        {"branch_trace", "numeric", "not_taken", 7, 3},
        {"branch_trace", "numeric", "taken", 6, 2},

        {"mixed_trace", "numeric", "not_taken", 10, 4},
        {"mixed_trace", "numeric", "not_taken", 10, 4},
        {"mixed_trace", "numeric", "not_taken", 10, 4},
        {"mixed_trace", "numeric", "not_taken", 10, 4}
    };
}

std::string most_common_string(const std::map<std::string, int>& counts) {
    std::string best;
    int best_count = -1;

    for (const auto& [value, count] : counts) {
        if (count > best_count) {
            best = value;
            best_count = count;
        }
    }

    return best;
}

int most_common_int(const std::map<int, int>& counts) {
    int best = -1;
    int best_count = -1;

    for (const auto& [value, count] : counts) {
        if (count > best_count) {
            best = value;
            best_count = count;
        }
    }

    return best;
}

std::map<std::string, TraceProfile> build_profiles(
    const std::vector<Invocation>& invocations,
    int hot_threshold
) {
    std::map<std::string, TraceProfile> profiles;
    std::map<std::string, std::map<std::string, int>> branch_counts;
    std::map<std::string, std::map<int, int>> block_counts;

    for (const auto& inv : invocations) {
        auto& profile = profiles[inv.trace_name];
        profile.trace_name = inv.trace_name;
        profile.invocation_count += 1;
        profile.opcode_count_total += inv.opcode_count;

        branch_counts[inv.trace_name][inv.branch_direction] += 1;
        block_counts[inv.trace_name][inv.basic_block_id] += 1;
    }

    for (auto& [name, profile] : profiles) {
        profile.hot_trace = profile.invocation_count >= hot_threshold;
        profile.dominant_branch_direction = most_common_string(branch_counts[name]);
        profile.dominant_basic_block = most_common_int(block_counts[name]);
        profile.repeated_branch_path_count =
            branch_counts[name][profile.dominant_branch_direction];
        profile.basic_block_reuse_count =
            block_counts[name][profile.dominant_basic_block];
    }

    return profiles;
}

std::vector<GuardCase> simulate_tiered_execution(
    const std::vector<Invocation>& invocations,
    const std::map<std::string, TraceProfile>& profiles,
    int hot_threshold
) {
    std::vector<GuardCase> cases;
    std::map<std::string, int> seen_count;

    for (const auto& inv : invocations) {
        seen_count[inv.trace_name] += 1;
        const auto& profile = profiles.at(inv.trace_name);

        GuardCase c;
        c.trace_name = inv.trace_name;
        c.invocation_index = seen_count[inv.trace_name];

        bool promoted = profile.hot_trace &&
                        seen_count[inv.trace_name] > hot_threshold;

        c.optimized_path_attempted = promoted;

        if (!promoted) {
            c.execution_tier = "baseline";
            c.optimized_path_taken = false;
            c.guard_failed = false;
            c.fallback_action = "none";
            cases.push_back(c);
            continue;
        }

        c.execution_tier = "optimized_candidate";

        bool numeric_guard_ok = inv.operand_type == "numeric";
        bool branch_guard_ok =
            profile.dominant_branch_direction == "none" ||
            inv.branch_direction == profile.dominant_branch_direction;
        bool block_guard_ok =
            inv.basic_block_id == profile.dominant_basic_block;

        if (!numeric_guard_ok) {
            c.guard_failed = true;
            c.guard_failure_reason = "operand-type-changed";
        } else if (!branch_guard_ok) {
            c.guard_failed = true;
            c.guard_failure_reason = "branch-direction-changed";
        } else if (!block_guard_ok) {
            c.guard_failed = true;
            c.guard_failure_reason = "basic-block-shape-changed";
        }

        if (c.guard_failed) {
            c.optimized_path_taken = false;
            c.execution_tier = "deoptimized-to-baseline";
            c.fallback_action = "fallback_to_baseline_execution";
        } else {
            c.optimized_path_taken = true;
            c.guard_failure_reason = "none";
            c.fallback_action = "none";
        }

        cases.push_back(c);
    }

    return cases;
}

std::vector<TieringSummary> summarize(
    const std::map<std::string, TraceProfile>& profiles,
    const std::vector<GuardCase>& cases,
    int hot_threshold
) {
    std::vector<TieringSummary> summaries;

    for (const auto& [name, profile] : profiles) {
        TieringSummary s;
        s.trace_name = name;
        s.total_invocations = profile.invocation_count;
        s.hot_threshold = hot_threshold;

        for (const auto& c : cases) {
            if (c.trace_name != name) {
                continue;
            }

            if (c.optimized_path_attempted) {
                s.optimized_attempts += 1;
            }

            if (c.optimized_path_taken) {
                s.optimized_hits += 1;
            }

            if (c.guard_failed) {
                s.deopt_count += 1;
            }
        }

        if (s.optimized_attempts > 0) {
            s.optimized_hit_rate =
                static_cast<double>(s.optimized_hits) /
                static_cast<double>(s.optimized_attempts);
        }

        if (!profile.hot_trace) {
            s.final_status = "baseline-only";
        } else if (s.deopt_count > 0) {
            s.final_status = "hot-with-deopt-events";
        } else {
            s.final_status = "hot-stable-optimized";
        }

        summaries.push_back(s);
    }

    std::sort(summaries.begin(), summaries.end(), [](const auto& a, const auto& b) {
        return a.trace_name < b.trace_name;
    });

    return summaries;
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

void write_profiles_json(
    const std::map<std::string, TraceProfile>& profiles,
    const std::string& path
) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"toy hot-trace profiling for tiered execution simulation; not a JIT or production runtime\",\n";
    out << "  \"profiles\": [\n";

    std::size_t i = 0;
    for (const auto& [name, p] : profiles) {
        out << "    {\n";
        out << "      \"trace_name\": \"" << json_escape(name) << "\",\n";
        out << "      \"invocation_count\": " << p.invocation_count << ",\n";
        out << "      \"opcode_count_total\": " << p.opcode_count_total << ",\n";
        out << "      \"repeated_branch_path_count\": " << p.repeated_branch_path_count << ",\n";
        out << "      \"basic_block_reuse_count\": " << p.basic_block_reuse_count << ",\n";
        out << "      \"dominant_branch_direction\": \"" << p.dominant_branch_direction << "\",\n";
        out << "      \"dominant_basic_block\": " << p.dominant_basic_block << ",\n";
        out << "      \"hot_trace\": " << (p.hot_trace ? "true" : "false") << "\n";
        out << "    }";

        if (++i != profiles.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
}

void write_guard_cases_json(
    const std::vector<GuardCase>& cases,
    const std::vector<TieringSummary>& summaries,
    const std::string& path
) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"toy speculative guard and deoptimization simulation; not a JIT or V8 implementation\",\n";
    out << "  \"guard_cases\": [\n";

    for (std::size_t i = 0; i < cases.size(); ++i) {
        const auto& c = cases[i];

        out << "    {\n";
        out << "      \"trace_name\": \"" << json_escape(c.trace_name) << "\",\n";
        out << "      \"invocation_index\": " << c.invocation_index << ",\n";
        out << "      \"optimized_path_attempted\": " << (c.optimized_path_attempted ? "true" : "false") << ",\n";
        out << "      \"optimized_path_taken\": " << (c.optimized_path_taken ? "true" : "false") << ",\n";
        out << "      \"guard_failed\": " << (c.guard_failed ? "true" : "false") << ",\n";
        out << "      \"guard_failure_reason\": \"" << json_escape(c.guard_failure_reason) << "\",\n";
        out << "      \"execution_tier\": \"" << json_escape(c.execution_tier) << "\",\n";
        out << "      \"fallback_action\": \"" << json_escape(c.fallback_action) << "\"\n";
        out << "    }";

        if (i + 1 != cases.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ],\n";
    out << "  \"summary\": [\n";

    for (std::size_t i = 0; i < summaries.size(); ++i) {
        const auto& s = summaries[i];

        out << "    {\n";
        out << "      \"trace_name\": \"" << json_escape(s.trace_name) << "\",\n";
        out << "      \"total_invocations\": " << s.total_invocations << ",\n";
        out << "      \"hot_threshold\": " << s.hot_threshold << ",\n";
        out << "      \"optimized_attempts\": " << s.optimized_attempts << ",\n";
        out << "      \"optimized_hits\": " << s.optimized_hits << ",\n";
        out << "      \"deopt_count\": " << s.deopt_count << ",\n";
        out << "      \"optimized_hit_rate\": " << std::fixed << std::setprecision(4)
            << s.optimized_hit_rate << ",\n";
        out << "      \"final_status\": \"" << json_escape(s.final_status) << "\"\n";
        out << "    }";

        if (i + 1 != summaries.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
}

void write_markdown(
    const std::map<std::string, TraceProfile>& profiles,
    const std::vector<GuardCase>& cases,
    const std::vector<TieringSummary>& summaries,
    const std::string& path
) {
    std::ofstream out(path);

    out << "# Tiering and Deoptimization Report\n\n";
    out << "> Scope: toy tiered-execution and deoptimization simulation for stack-bytecode traces. This is not a JIT, V8 engine, WebAssembly runtime, or production compiler implementation.\n\n";

    out << "## Hot trace profiles\n\n";
    out << "| Trace | Invocations | Opcode Count | Repeated Branch Path | Basic Block Reuse | Dominant Branch | Hot Trace |\n";
    out << "|---|---:|---:|---:|---:|---|---|\n";

    for (const auto& [name, p] : profiles) {
        out << "| " << name
            << " | " << p.invocation_count
            << " | " << p.opcode_count_total
            << " | " << p.repeated_branch_path_count
            << " | " << p.basic_block_reuse_count
            << " | " << p.dominant_branch_direction
            << " | " << (p.hot_trace ? "true" : "false")
            << " |\n";
    }

    out << "\n## Tiering summary\n\n";
    out << "| Trace | Optimized Attempts | Optimized Hits | Deopts | Hit Rate | Final Status |\n";
    out << "|---|---:|---:|---:|---:|---|\n";

    for (const auto& s : summaries) {
        out << "| " << s.trace_name
            << " | " << s.optimized_attempts
            << " | " << s.optimized_hits
            << " | " << s.deopt_count
            << " | " << std::fixed << std::setprecision(2)
            << (s.optimized_hit_rate * 100.0) << "%"
            << " | " << s.final_status
            << " |\n";
    }

    out << "\n## Guard failures and fallback events\n\n";
    out << "| Trace | Invocation | Tier | Optimized Attempted | Optimized Taken | Guard Failed | Reason | Fallback |\n";
    out << "|---|---:|---|---|---|---|---|---|\n";

    for (const auto& c : cases) {
        if (!c.optimized_path_attempted && !c.guard_failed) {
            continue;
        }

        out << "| " << c.trace_name
            << " | " << c.invocation_index
            << " | " << c.execution_tier
            << " | " << (c.optimized_path_attempted ? "true" : "false")
            << " | " << (c.optimized_path_taken ? "true" : "false")
            << " | " << (c.guard_failed ? "true" : "false")
            << " | " << c.guard_failure_reason
            << " | " << c.fallback_action
            << " |\n";
    }

    out << "\n## Interpretation\n\n";
    out << "- Hot traces are detected from repeated invocation counts, repeated branch paths, and basic-block reuse.\n";
    out << "- Optimized-path execution is attempted only after a trace exceeds the hot threshold.\n";
    out << "- Guard checks model speculative assumptions such as stable numeric operands, stable branch direction, and stable basic-block shape.\n";
    out << "- Guard failures trigger fallback to baseline execution and are recorded as deoptimization events.\n";
    out << "- This demonstrates runtime feedback and optimization tradeoff reasoning without claiming production JIT/compiler implementation experience.\n";
}

int main() {
    constexpr int hot_threshold = 3;

    const auto invocations = build_invocations();
    const auto profiles = build_profiles(invocations, hot_threshold);
    const auto guard_cases = simulate_tiered_execution(
        invocations,
        profiles,
        hot_threshold
    );
    const auto summaries = summarize(profiles, guard_cases, hot_threshold);

    write_profiles_json(
        profiles,
        "execution_engine_lab/hot_trace_profiles.json"
    );
    write_guard_cases_json(
        guard_cases,
        summaries,
        "execution_engine_lab/speculative_guard_cases.json"
    );
    write_markdown(
        profiles,
        guard_cases,
        summaries,
        "execution_engine_lab/tiering_and_deopt_report.md"
    );

    std::cout << "Generated execution_engine_lab/hot_trace_profiles.json\n";
    std::cout << "Generated execution_engine_lab/speculative_guard_cases.json\n";
    std::cout << "Generated execution_engine_lab/tiering_and_deopt_report.md\n";

    return 0;
}
