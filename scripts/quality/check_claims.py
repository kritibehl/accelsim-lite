from pathlib import Path
import sys

BANNED = [
    "accurate gpu simulator",
    "matched nvidia",
    "production gpu runtime optimization",
    "production llm inference optimization",
]

CONTEXTUAL_BANNED = [
    "cycle-accurate gpu simulator",
    "cuda kernel execution",
    "tensor core simulation",
    "warp scheduling",
    "hardware-accurate",
    "real mlperf",
]

ALLOWLIST_FILES = {
    "docs/claims_boundary.md",
}

ALLOWLIST_CONTEXT_MARKERS = [
    "do not claim",
    "does not model",
    "not a",
    "not calibrated",
    "intentionally does not",
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
    text = path.read_text(errors="ignore")
    lowered = text.lower()

    if rel in ALLOWLIST_FILES:
        continue

    for phrase in BANNED:
        if phrase in lowered:
            violations.append((rel, phrase))

    lines = lowered.splitlines()
    for i, line in enumerate(lines):
        for phrase in CONTEXTUAL_BANNED:
            if phrase in line:
                window = " ".join(lines[max(0, i - 2): i + 3])
                if not any(marker in window for marker in ALLOWLIST_CONTEXT_MARKERS):
                    violations.append((rel, phrase))

if violations:
    print("Potential inflated claims found:\n")
    for rel, phrase in violations:
        print(f"- {rel}: {phrase}")
    sys.exit(1)

print("Claims check passed.")
