"""Every phone NormalizeText emits for the test cases must exist in the model's symbol table.

This is the contract ADR-049 promised and that nothing enforced: a token outside the inventory is
silently dropped by the engine (phones_to_ids), which is how «r» lost its vowel after the app moved
to the 22 kHz model.

Usage: python3 tools/check_inventory_coverage.py --symbols <model symbols.txt> [--cases tests/phoneme_cases.tsv]
Exit code 0 = every emitted token is in the inventory.
"""
import argparse
import os
import subprocess
import sys
import tempfile


def load_cases(path):
    cases = []
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) >= 2:
                cases.append((parts[0], parts[1]))
    return cases


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="build/NormalizeCSV")
    ap.add_argument("--symbols", required=True)
    ap.add_argument("--cases", default="tests/phoneme_cases.tsv")
    args = ap.parse_args()

    known = set()
    with open(args.symbols, encoding="utf-8") as fh:
        for line in fh:
            line = line.strip()
            if line:
                known.add(line.lstrip("@"))

    cases = load_cases(args.cases)
    by_lang = {"FA": [], "EN": []}
    for text, phones in cases:
        by_lang["EN" if phones.startswith("en:") else "FA"].append(text)

    missing = []
    checked = 0
    for lang, texts in by_lang.items():
        if not texts:
            continue
        with tempfile.TemporaryDirectory() as tmp:
            src = os.path.join(tmp, "cases.txt")
            with open(src, "w", encoding="utf-8") as fh:
                for i, t in enumerate(texts):
                    fh.write(f"C{i:04d}|{t}\n")
            subprocess.run([args.bin, lang, src, "--grain", "--symbols", args.symbols],
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            with open(src + "-grain.csv", encoding="utf-8") as fh:
                for line in fh:
                    parts = line.rstrip("\n").split("|")
                    if len(parts) < 2:
                        continue
                    for tok in parts[1].split():
                        checked += 1
                        if tok not in known:
                            missing.append((lang, parts[0].split("/")[-1], tok))

    print(f"checked {checked} emitted tokens against {len(known)} inventory symbols")
    if missing:
        for lang, cid, tok in missing[:40]:
            print(f"  MISSING {lang} {cid}: {tok}")
        print(f"inventory coverage: {len(missing)} token(s) outside the table")
        return 1
    print("inventory coverage: every emitted token is in the table")
    return 0


if __name__ == "__main__":
    sys.exit(main())
