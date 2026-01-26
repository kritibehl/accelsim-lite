import csv, sys
from collections import Counter

if len(sys.argv) < 2:
    print("Usage: python summarize_trace.py <trace_out.csv>")
    sys.exit(1)

events = []
with open(sys.argv[1], newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        events.append(row)

if not events:
    print("Empty trace.")
    sys.exit(0)

last_cycle = max(int(e["cycle"]) for e in events)
cnt = Counter(e["event"] for e in events)
print("Total cycles:", last_cycle + 1)
print("Event counts:", dict(cnt))
