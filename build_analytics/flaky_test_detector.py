#!/usr/bin/env python3
import json
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent
HISTORY = ROOT / "build_history.json"
TRENDS = ROOT / "regression_trends.json"
REPORT = ROOT / "build_health_report.md"

data = json.loads(HISTORY.read_text())
builds = data["builds"]

failed_builds = [b for b in builds if b["status"] == "fail"]
passed_builds = [b for b in builds if b["status"] == "pass"]

test_failures = Counter()
test_seen_in_failed_builds = defaultdict(list)
root_causes = Counter()

for b in failed_builds:
    for t in b["failed_tests"]:
        test_failures[t] += 1
        test_seen_in_failed_builds[t].append(b["build_id"])
    if b["root_cause"]:
        root_causes[b["root_cause"]] += 1

flaky_tests = [
    {
        "test": test,
        "failure_count": count,
        "builds": test_seen_in_failed_builds[test],
        "classification": "flaky_or_recurring" if count >= 2 else "single_failure"
    }
    for test, count in test_failures.items()
    if count >= 2
]

avg_pass_duration = sum(b["duration_sec"] for b in passed_builds) / len(passed_builds)
avg_fail_duration = sum(b["duration_sec"] for b in failed_builds) / len(failed_builds)

top_root_cause, top_count = root_causes.most_common(1)[0]

summary = {
    "scope": "synthetic build analytics for CI/build health review",
    "total_builds": len(builds),
    "failed_builds": len(failed_builds),
    "passed_builds": len(passed_builds),
    "failure_rate_percent": round(100 * len(failed_builds) / len(builds), 2),
    "flaky_tests": len(flaky_tests),
    "flaky_test_details": flaky_tests,
    "regression_detected": bool(flaky_tests or len(failed_builds) >= 3),
    "root_cause": top_root_cause,
    "root_cause_occurrences": top_count,
    "avg_pass_duration_sec": round(avg_pass_duration, 2),
    "avg_fail_duration_sec": round(avg_fail_duration, 2),
    "duration_delta_fail_vs_pass_sec": round(avg_fail_duration - avg_pass_duration, 2)
}

TRENDS.write_text(json.dumps(summary, indent=2))

lines = [
    "# Build Health Report",
    "",
    "> Scope: synthetic CI/build analytics over AccelSim-Lite build-history data.",
    "",
    "## Summary",
    "",
    "```json",
    json.dumps({
        "failed_builds": summary["failed_builds"],
        "flaky_tests": summary["flaky_tests"],
        "regression_detected": summary["regression_detected"],
        "root_cause": summary["root_cause"]
    }, indent=2),
    "```",
    "",
    "## Build health metrics",
    "",
    f"- Total builds analyzed: {summary['total_builds']}",
    f"- Failed builds: {summary['failed_builds']}",
    f"- Failure rate: {summary['failure_rate_percent']}%",
    f"- Average pass duration: {summary['avg_pass_duration_sec']} sec",
    f"- Average fail duration: {summary['avg_fail_duration_sec']} sec",
    f"- Failure-duration delta: {summary['duration_delta_fail_vs_pass_sec']} sec",
    "",
    "## Flaky / recurring tests",
    "",
]

if flaky_tests:
    for item in flaky_tests:
        lines.append(
            f"- `{item['test']}` failed {item['failure_count']} times "
            f"across builds {', '.join(item['builds'])}"
        )
else:
    lines.append("- No recurring flaky tests detected.")

lines += [
    "",
    "## Root-cause trend",
    "",
    f"- Top recurring root cause: `{summary['root_cause']}` ({summary['root_cause_occurrences']} occurrences)",
    "",
    "## What this means",
    "",
    "The build-history data shows recurring test failures and a dominant cache-invalidation root cause. This models CI/build-health workflows used to identify regressions, flaky tests, and recurring infrastructure failure modes.",
    "",
    "## Claims boundary",
    "",
    "This is a synthetic build-analytics workflow for CI/CD validation practice. It does not claim production build-system ownership or Apple internal infrastructure experience."
]

REPORT.write_text("\n".join(lines) + "\n")

print(json.dumps({
    "failed_builds": summary["failed_builds"],
    "flaky_tests": summary["flaky_tests"],
    "regression_detected": summary["regression_detected"],
    "root_cause": summary["root_cause"]
}, indent=2))
