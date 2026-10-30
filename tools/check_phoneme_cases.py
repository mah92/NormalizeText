#!/usr/bin/env python3
"""Run the phoneme regression cases through NormalizeCSV and diff the result.

Usage:
    python3 tools/check_phoneme_cases.py [--bin build/NormalizeCSV] [--lex DIR]

Exit code 0 = all cases match, 1 = mismatch (a unified diff is printed).
The cases live in tests/phoneme_cases.tsv (lang, input, expected phones) and are
the single source of truth shared with the Android instrumented test.
"""
import argparse
import os
import subprocess
import sys
import tempfile


def read_cases(path):
    cases = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) != 3:
                raise SystemExit(f"bad case line: {line!r}")
            cases.append(tuple(parts))
    return cases


def run_batch(binary, lex, lang, texts):
    with tempfile.TemporaryDirectory() as tmp:
        src = os.path.join(tmp, "cases.txt")
        with open(src, "w", encoding="utf-8") as f:
            for i, t in enumerate(texts):
                f.write(f"C{i:04d}|{t}\n")
        subprocess.run(
            [binary, lang, src, "--grain", lex],
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
        )
        out = {}
        with open(src + "-grain.csv", encoding="utf-8") as f:
            for line in f:
                a = line.rstrip("\n").split("|")
                if len(a) > 1:
                    out[a[0].split("/")[-1].replace(".wav", "")] = a[1]
        return [out.get(f"C{i:04d}", "") for i in range(len(texts))]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", default="build/NormalizeCSV")
    ap.add_argument("--lex", default=os.path.expanduser(
        "~/Basir/TTS/GrainSpeech/lex_matcha"))
    ap.add_argument("--cases", default="tests/phoneme_cases.tsv")
    args = ap.parse_args()

    cases = read_cases(args.cases)
    got = {}
    for lang in sorted({c[0] for c in cases}):
        texts = [c[1] for c in cases if c[0] == lang]
        for t, p in zip(texts, run_batch(args.bin, args.lex, lang, texts)):
            got[(lang, t)] = p

    bad = 0
    for lang, text, want in cases:
        have = got.get((lang, text), "")
        if have != want:
            bad += 1
            print(f"FAIL [{lang}] {text!r}\n  want: {want}\n  got : {have}")
    print(f"\n{len(cases) - bad}/{len(cases)} cases passed")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
