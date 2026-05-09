from pathlib import Path
import sys

BANNED_ALWAYS = [
    "matched nvidia",
    "production gpu runtime optimization",
    "production llm inference optimization",
]

CONTEXTUAL_BANNED = [
    "accurate gpu simulator",
    "cycle-accurate gpu simulator",
    "cuda kernel execution",
    "tensor core simulation",
    "warp scheduling",
    "hardware-accurate",
    "real mlperf",
]

ALLOWLIST_FILES = {
    "docs/claims_boundary.md",
    "docs/adr/0001-workload-level-not-cycle-accurate.md",
}

SAFE_CONTEXT = [
    "do not claim",
    "does not claim",
    "does not model",
    "not a",
    "not calibrated",
    "intentionally does not",
    "unsafe claim",
    "avoid this",
    "does not",
]

paths = [
    p for p in Path(".").rglob("*")
    if p.is_file()
    and ".git" not in p.parts
    and p.suffix.lower() in {".md", ".txt", ".tex"}
]

violations = []

for path in paths:
    rel = str(path)
    if rel in ALLOWLIST_FILES:
        continue

    lines = path.read_text(errors="ignore").lower().splitlines()

    for i, line in enumerate(lines):
        window = " ".join(lines[max(0, i - 3): i + 4])

        for phrase in BANNED_ALWAYS:
            if phrase in line and not any(marker in window for marker in SAFE_CONTEXT):
                violations.append((rel, phrase))

        for phrase in CONTEXTUAL_BANNED:
            if phrase in line and not any(marker in window for marker in SAFE_CONTEXT):
                violations.append((rel, phrase))

if violations:
    print("Potential inflated claims found:\n")
    for rel, phrase in violations:
        print(f"- {rel}: {phrase}")
    sys.exit(1)

print("Claims check passed.")
