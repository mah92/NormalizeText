#!/usr/bin/env bash
# Persian number-format regression test.
#
# Feeds tests/persian_number_formats.tsv through the CLI and checks, per case, that the NORMALIZED
# text contains the expected fragment (and, when a third column is present, that the IPA does too).
#
# usage: tests/run_format_tests.sh [path-to-NormalizeCSV]     (default: ../build/NormalizeCSV)
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${1:-$HERE/../build/NormalizeCSV}"
[ -x "$BIN" ] || { echo "no CLI at $BIN — build it first: cd build && make NormalizeCSV"; exit 2; }

TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
grep -v '^#' "$HERE/persian_number_formats.tsv" | cut -f1 | nl -ba -w4 -s'|' > "$TMP/in.txt"
"$BIN" FA "$TMP/in.txt" --grain > /dev/null 2>&1 || { echo "CLI run failed"; exit 2; }

python3 - "$TMP/in.txt-complete.csv" "$HERE/persian_number_formats.tsv" <<'PY'
import io, sys

rows = []
for line in io.open(sys.argv[2], encoding="utf-8"):
    if not line.strip() or line.startswith("#"):
        continue
    cols = line.rstrip("\n").split("\t")
    rows.append((cols[0].strip(), cols[1].strip(), cols[2].strip() if len(cols) > 2 else None))

got = {}
for line in io.open(sys.argv[1], encoding="utf-8"):
    cols = line.rstrip("\n").split("|")
    if len(cols) >= 5:                      # stem|?|original|normalized|ipa
        key = cols[0].split("/")[-1].replace(".wav", "").strip()
        got[key] = (cols[3].strip(), cols[4].strip())

ok = bad = 0
for i, (inp, want, want_ipa) in enumerate(rows, 1):
    norm, ipa = got.get(str(i), ("<missing>", ""))
    problems = []
    if want not in norm:
        problems.append(f"normalized: want {want!r}, got {norm!r}")
    if want_ipa and want_ipa not in ipa:
        problems.append(f"ipa: want {want_ipa!r}, got {ipa!r}")
    if problems:
        bad += 1
        print(f"FAIL [{i}] {inp!r}")
        for p in problems:
            print("      " + p)
    else:
        ok += 1
print(f"persian number formats: {ok} passed, {bad} failed")
sys.exit(1 if bad else 0)
PY
