with open('utf8_chars-final.txt', 'r', encoding='utf-8') as f:
    lines = f.read().splitlines()

# Remove duplicates and sort by Unicode code point
unique_sorted = sorted(set(lines), key=lambda x: ord(x[0]) if x else 0)

with open('utf8_chars-final2.txt', 'w', encoding='utf-8') as f:
    f.write('\n'.join(unique_sorted) + '\n')
