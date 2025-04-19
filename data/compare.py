with open('dataset_AR-EN.txt-result.txt', 'r') as f1, open('dataset_AR-EN.txt-normalized.txt', 'r') as f2:
#with open('dataset_FA-EN-AR.txt-result.txt', 'r', encoding='utf-8', errors='ignore') as f1, open('dataset_FA-EN-AR.txt-normalized.txt', 'r', encoding='utf-8', errors='ignore') as f2:
    lines1 = f1.readlines()
    lines2 = f2.readlines()

max_lines = max(len(lines1), len(lines2))
for i in range(max_lines):
    line1 = lines1[i].strip() if i < len(lines1) else None
    line2 = lines2[i].strip() if i < len(lines2) else None
    if line1 != line2:
        #print(f"Line {i+1}: File1='{line1}', File2='{line2}'")
        #print(f"{i+1}")
        print(f"{line2}")