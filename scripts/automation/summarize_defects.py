from pathlib import Path

reports = list(Path("reports/analysis").glob("*.md"))

print("=== Analysis Reports ===")

for report in reports:
    print(report)
