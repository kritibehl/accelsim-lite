import json

with open("reports/latest/summary.json") as f:
    data = json.load(f)

print("\n=== PERFORMANCE EXPLANATION ===\n")

if data["top_bottleneck"] == "NoMemoryPort":
    print("Performance limited by memory bandwidth.")
    print("→ Increase memory ports or reduce memory-heavy ops.")

elif data["top_bottleneck"] == "WaitingDependency":
    print("Performance limited by instruction dependencies.")
    print("→ Parallelize workload or reduce dependency chains.")

elif data["top_bottleneck"] == "NoComputeUnit":
    print("Performance limited by compute resources.")
    print("→ Increase compute units or reduce contention.")

elif data["top_bottleneck"] == "DispatchQueueFull":
    print("Performance limited by queue capacity.")
    print("→ Increase dispatch queue size.")

else:
    print("Mixed bottlenecks detected.")
