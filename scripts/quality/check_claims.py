from pathlib import Path
import sys

BANNED = [
    "cycle-accurate GPU simulator",
    "accurate GPU simulator",
    "CUDA kernel execution",
    "Tensor Core simulation",
    "warp scheduling simulation",
    "SM-level occupancy",
    "hardware-accurate",
    "matched NVIDIA",
    "real MLPerf",
    "production GPU runtime optimization",
    "production LLM inference optimization",
]

ALLOWLIST = {
    "docs/claims_boundary.md",
}

paths = [
    p for p in Path(".").rglob("*")
    if p.is_file()
    and ".git" not in p.parts
    and p.suffix.lower() in {".md", ".txt", ".tex"}
]

violations = []

for path in paths:
    rel = str(path)
    text = path.read_text(errors="ignore")
    for phrase in BANNED:
        if phrase.lower() in text.lower() and rel not in ALLOWLIST:
            violations.append((rel, phrase))

if violations:
    print("Potential inflated claims found:\n")
    for rel, phrase in violations:
        print(f"- {rel}: {phrase}")
    sys.exit(1)

print("Claims check passed.")
