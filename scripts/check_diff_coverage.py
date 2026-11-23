"""Report C++ line coverage and enforce coverage of changed executable lines."""

import argparse
from pathlib import Path
import re
import subprocess


def read_coverage(report, root):
    coverage = {}
    source = None
    for line in report.read_text().splitlines():
        if line.startswith("SF:"):
            path = (root / line[3:]).resolve()
            if path.is_relative_to(root):
                relative = path.relative_to(root)
                if relative.parts[0] in {"src", "include"}:
                    source = coverage.setdefault(relative.as_posix(), {})
                    continue
            source = None
        elif line.startswith("DA:") and source is not None:
            number, hits, *_ = line[3:].split(",")
            number = int(number)
            source[number] = source.get(number, False) or int(hits) > 0
    return coverage


def changed_lines(base, root):
    diff = subprocess.check_output(
        ["git", "diff", "--no-ext-diff", "--no-color", "--unified=0", base, "HEAD", "--", "src", "include"],
        cwd=root,
        text=True,
    )
    path = None
    for line in diff.splitlines():
        if line.startswith("+++ "):
            path = line[6:] if line.startswith("+++ b/") else None
        elif path and (match := re.match(r"^@@ .* \+(\d+)(?:,(\d+))? @@", line)):
            start = int(match[1])
            for number in range(start, start + int(match[2] or 1)):
                yield path, number


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path)
    parser.add_argument("--base", help="Base commit for the changed-line gate")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    coverage = read_coverage(args.report, root)
    total = sum(len(lines) for lines in coverage.values())
    if not total:
        parser.exit(1, "No production lines found in coverage report.\n")
    covered = sum(sum(lines.values()) for lines in coverage.values())
    print(f"Project line coverage: {covered / total:.2%} ({covered}/{total})")
    if not args.base:
        return

    changed = set(changed_lines(args.base, root))
    missing_sources = sorted({path for path, _ in changed if path.endswith(".cpp") and path not in coverage})
    if missing_sources:
        parser.exit(1, "Changed source files missing from coverage report: " + ", ".join(missing_sources) + "\n")
    executable = [(path, line) for path, line in sorted(changed) if line in coverage.get(path, {})]
    if not executable:
        print("No changed executable lines.")
        return

    missed = [(path, line) for path, line in executable if not coverage[path][line]]
    covered = len(executable) - len(missed)
    print(f"Changed line coverage: {covered / len(executable):.2%} ({covered}/{len(executable)})")
    if covered * 100 < 90 * len(executable):
        for path, line in missed:
            print(f"Uncovered: {path}:{line}")
        parser.exit(1, "Changed line coverage is below 90%.\n")


if __name__ == "__main__":
    main()
