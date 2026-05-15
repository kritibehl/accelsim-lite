#include <algorithm>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

enum class TaskKind {
    CpuBound,
    IoBound
};

struct Task {
    int id;
    std::string name;
    TaskKind kind;
    int arrival_time;
    int service_time;
    int priority;
};

struct TaskResult {
    int task_id;
    std::string name;
    std::string kind;
    int priority;
    int arrival_time;
    int start_time;
    int finish_time;
    int wait_time;
    int turnaround_time;
};

struct PolicyResult {
    std::string policy;
    std::vector<TaskResult> tasks;
    int total_time = 0;
    double avg_wait_time = 0.0;
    double avg_turnaround_time = 0.0;
    double throughput = 0.0;
    int max_wait_time = 0;
    double fairness_index = 0.0;
    std::string starvation_risk;
};

std::string kind_to_string(TaskKind kind) {
    return kind == TaskKind::CpuBound ? "cpu_bound" : "io_bound";
}

std::vector<Task> make_contention_workload() {
    return {
        {0, "cpu-render-0", TaskKind::CpuBound, 0, 14, 2},
        {1, "io-fetch-0", TaskKind::IoBound, 1, 4, 1},
        {2, "cpu-compile-1", TaskKind::CpuBound, 2, 12, 3},
        {3, "io-fetch-1", TaskKind::IoBound, 3, 3, 1},
        {4, "cpu-analysis-2", TaskKind::CpuBound, 4, 10, 2},
        {5, "io-sync-2", TaskKind::IoBound, 5, 5, 1},
        {6, "priority-control", TaskKind::IoBound, 6, 2, 0},
        {7, "cpu-batch-3", TaskKind::CpuBound, 7, 16, 4}
    };
}

double jains_fairness_index(const std::vector<int>& waits) {
    if (waits.empty()) {
        return 1.0;
    }

    double sum = std::accumulate(waits.begin(), waits.end(), 0.0);
    double sum_sq = 0.0;

    for (int wait : waits) {
        sum_sq += static_cast<double>(wait) * static_cast<double>(wait);
    }

    if (sum_sq == 0.0) {
        return 1.0;
    }

    return (sum * sum) / (static_cast<double>(waits.size()) * sum_sq);
}

PolicyResult finalize_result(const std::string& policy,
                             std::vector<TaskResult> results) {
    PolicyResult out;
    out.policy = policy;
    out.tasks = results;

    std::vector<int> waits;

    for (const auto& task : results) {
        out.total_time = std::max(out.total_time, task.finish_time);
        out.avg_wait_time += task.wait_time;
        out.avg_turnaround_time += task.turnaround_time;
        out.max_wait_time = std::max(out.max_wait_time, task.wait_time);
        waits.push_back(task.wait_time);
    }

    if (!results.empty()) {
        out.avg_wait_time /= static_cast<double>(results.size());
        out.avg_turnaround_time /= static_cast<double>(results.size());
        out.throughput = static_cast<double>(results.size()) /
                         static_cast<double>(std::max(1, out.total_time));
    }

    out.fairness_index = jains_fairness_index(waits);
    out.starvation_risk =
        out.max_wait_time > 30 || out.fairness_index < 0.55
            ? "elevated"
            : "low";

    return out;
}

PolicyResult run_fifo(std::vector<Task> tasks) {
    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return a.arrival_time < b.arrival_time;
    });

    int now = 0;
    std::vector<TaskResult> results;

    for (const auto& task : tasks) {
        now = std::max(now, task.arrival_time);
        int start = now;
        int finish = start + task.service_time;
        now = finish;

        results.push_back({
            task.id,
            task.name,
            kind_to_string(task.kind),
            task.priority,
            task.arrival_time,
            start,
            finish,
            start - task.arrival_time,
            finish - task.arrival_time
        });
    }

    return finalize_result("fifo", results);
}

PolicyResult run_priority(std::vector<Task> tasks) {
    int now = 0;
    std::vector<TaskResult> results;
    std::vector<bool> done(tasks.size(), false);

    while (results.size() < tasks.size()) {
        int chosen = -1;

        for (std::size_t i = 0; i < tasks.size(); ++i) {
            if (done[i] || tasks[i].arrival_time > now) {
                continue;
            }

            if (chosen == -1 ||
                tasks[i].priority < tasks[chosen].priority ||
                (tasks[i].priority == tasks[chosen].priority &&
                 tasks[i].arrival_time < tasks[chosen].arrival_time)) {
                chosen = static_cast<int>(i);
            }
        }

        if (chosen == -1) {
            now += 1;
            continue;
        }

        const auto& task = tasks[chosen];
        int start = now;
        int finish = start + task.service_time;
        now = finish;
        done[chosen] = true;

        results.push_back({
            task.id,
            task.name,
            kind_to_string(task.kind),
            task.priority,
            task.arrival_time,
            start,
            finish,
            start - task.arrival_time,
            finish - task.arrival_time
        });
    }

    return finalize_result("priority", results);
}

PolicyResult run_round_robin(std::vector<Task> tasks, int quantum) {
    int now = 0;
    std::deque<Task> ready;
    std::map<int, int> remaining;
    std::map<int, int> first_start;
    std::map<int, int> finish_time;
    std::vector<Task> ordered = tasks;

    std::sort(ordered.begin(), ordered.end(), [](const Task& a, const Task& b) {
        return a.arrival_time < b.arrival_time;
    });

    for (const auto& task : ordered) {
        remaining[task.id] = task.service_time;
        first_start[task.id] = -1;
    }

    std::size_t next_arrival = 0;

    auto enqueue_arrivals = [&]() {
        while (next_arrival < ordered.size() &&
               ordered[next_arrival].arrival_time <= now) {
            ready.push_back(ordered[next_arrival]);
            next_arrival += 1;
        }
    };

    while (finish_time.size() < ordered.size()) {
        enqueue_arrivals();

        if (ready.empty()) {
            now += 1;
            continue;
        }

        Task task = ready.front();
        ready.pop_front();

        if (first_start[task.id] == -1) {
            first_start[task.id] = now;
        }

        int slice = std::min(quantum, remaining[task.id]);
        now += slice;
        remaining[task.id] -= slice;

        enqueue_arrivals();

        if (remaining[task.id] > 0) {
            ready.push_back(task);
        } else {
            finish_time[task.id] = now;
        }
    }

    std::vector<TaskResult> results;

    for (const auto& task : ordered) {
        int finish = finish_time[task.id];
        int turnaround = finish - task.arrival_time;
        int wait = turnaround - task.service_time;

        results.push_back({
            task.id,
            task.name,
            kind_to_string(task.kind),
            task.priority,
            task.arrival_time,
            first_start[task.id],
            finish,
            wait,
            turnaround
        });
    }

    return finalize_result("round_robin_q4", results);
}

std::string json_escape(const std::string& input) {
    std::ostringstream out;

    for (char c : input) {
        if (c == '"') {
            out << "\\\"";
        } else {
            out << c;
        }
    }

    return out.str();
}

void write_json(const std::vector<PolicyResult>& results,
                const std::string& path) {
    std::ofstream out(path);

    out << "{\n";
    out << "  \"scope\": \"synthetic workload scheduling experiment; not an OS scheduler benchmark\",\n";
    out << "  \"policies\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& policy = results[i];

        out << "    {\n";
        out << "      \"policy\": \"" << policy.policy << "\",\n";
        out << "      \"total_time\": " << policy.total_time << ",\n";
        out << "      \"avg_wait_time\": " << std::fixed << std::setprecision(4) << policy.avg_wait_time << ",\n";
        out << "      \"avg_turnaround_time\": " << std::fixed << std::setprecision(4) << policy.avg_turnaround_time << ",\n";
        out << "      \"throughput\": " << std::fixed << std::setprecision(6) << policy.throughput << ",\n";
        out << "      \"max_wait_time\": " << policy.max_wait_time << ",\n";
        out << "      \"fairness_index\": " << std::fixed << std::setprecision(4) << policy.fairness_index << ",\n";
        out << "      \"starvation_risk\": \"" << policy.starvation_risk << "\",\n";
        out << "      \"tasks\": [\n";

        for (std::size_t j = 0; j < policy.tasks.size(); ++j) {
            const auto& task = policy.tasks[j];

            out << "        {\n";
            out << "          \"task_id\": " << task.task_id << ",\n";
            out << "          \"name\": \"" << json_escape(task.name) << "\",\n";
            out << "          \"kind\": \"" << task.kind << "\",\n";
            out << "          \"priority\": " << task.priority << ",\n";
            out << "          \"arrival_time\": " << task.arrival_time << ",\n";
            out << "          \"start_time\": " << task.start_time << ",\n";
            out << "          \"finish_time\": " << task.finish_time << ",\n";
            out << "          \"wait_time\": " << task.wait_time << ",\n";
            out << "          \"turnaround_time\": " << task.turnaround_time << "\n";
            out << "        }";

            if (j + 1 != policy.tasks.size()) {
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

void write_markdown(const std::vector<PolicyResult>& results,
                    const std::string& path) {
    std::ofstream out(path);

    out << "# Scheduler Trace Report\n\n";
    out << "> Scope: synthetic workload scheduling experiment comparing FIFO, round-robin, and priority scheduling. This is not an OS scheduler benchmark.\n\n";

    out << "| Policy | Total Time | Avg Wait | Avg Turnaround | Throughput | Max Wait | Fairness Index | Starvation Risk |\n";
    out << "|---|---:|---:|---:|---:|---:|---:|---|\n";

    for (const auto& policy : results) {
        out << "| " << policy.policy
            << " | " << policy.total_time
            << " | " << std::fixed << std::setprecision(2) << policy.avg_wait_time
            << " | " << std::fixed << std::setprecision(2) << policy.avg_turnaround_time
            << " | " << std::fixed << std::setprecision(6) << policy.throughput
            << " | " << policy.max_wait_time
            << " | " << std::fixed << std::setprecision(4) << policy.fairness_index
            << " | " << policy.starvation_risk << " |\n";
    }

    out << "\n## Scheduling traces\n\n";

    for (const auto& policy : results) {
        out << "### " << policy.policy << "\n\n";
        out << "| Task | Kind | Priority | Arrival | Start | Finish | Wait | Turnaround |\n";
        out << "|---|---|---:|---:|---:|---:|---:|---:|\n";

        for (const auto& task : policy.tasks) {
            out << "| " << task.name
                << " | " << task.kind
                << " | " << task.priority
                << " | " << task.arrival_time
                << " | " << task.start_time
                << " | " << task.finish_time
                << " | " << task.wait_time
                << " | " << task.turnaround_time
                << " |\n";
        }

        out << "\n";
    }

    out << "## Interpretation\n\n";
    out << "- FIFO preserves arrival order but can increase wait time for short I/O-bound tasks behind long CPU-bound tasks.\n";
    out << "- Round-robin improves response distribution but can increase turnaround for CPU-heavy tasks depending on quantum size.\n";
    out << "- Priority scheduling improves latency for high-priority I/O/control tasks but can increase starvation risk for lower-priority CPU-bound tasks.\n";
    out << "- The experiment is intended for workload-management reasoning, not OS-kernel scheduler benchmarking.\n";
}

int main() {
    const auto workload = make_contention_workload();

    std::vector<PolicyResult> results;
    results.push_back(run_fifo(workload));
    results.push_back(run_round_robin(workload, 4));
    results.push_back(run_priority(workload));

    write_json(results, "scheduler_lab/fairness_latency_comparison.json");
    write_markdown(results, "scheduler_lab/scheduler_trace_report.md");

    std::cout << "Generated scheduler_lab/fairness_latency_comparison.json\n";
    std::cout << "Generated scheduler_lab/scheduler_trace_report.md\n";

    return 0;
}
