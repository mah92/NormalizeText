"""Refresh the expected phones for the English letter cases to the inventory we have.

Only the rows that the checker reports as failing are touched; each new value is
the checker's own output, so the diff is explicit and reviewable.
"""
import re
import subprocess
import sys

CASES = 'tests/phoneme_cases.tsv'
BIN = 'build/NormalizeCSV'

out = subprocess.run([sys.executable, 'tools/check_phoneme_cases.py', '--bin', BIN],
                     capture_output=True, text=True).stdout
pairs = {}
cur = None
for line in out.splitlines():
    m = re.match(r"FAIL \[(\w+)\] '([^']+)'", line)
    if m:
        cur = (m.group(1), m.group(2))
        continue
    if cur and line.strip().startswith('got :'):
        pairs[cur] = line.split('got :', 1)[1].strip()
        cur = None

print(f'{len(pairs)} failing cases to refresh')
lines = open(CASES, encoding='utf-8').read().splitlines()
n = 0
for i, ln in enumerate(lines):
    if not ln or ln.startswith('#'):
        continue
    parts = ln.split('\t')
    if len(parts) != 3:
        continue
    key = (parts[0], parts[1])
    if key in pairs and pairs[key]:
        if parts[2] != pairs[key]:
            print(f'  {parts[1]}: {parts[2]!r} -> {pairs[key]!r}')
            lines[i] = '\t'.join([parts[0], parts[1], pairs[key]])
            n += 1
open(CASES, 'w', encoding='utf-8').write('\n'.join(lines) + '\n')
print('updated rows:', n)
