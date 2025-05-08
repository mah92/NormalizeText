with open('other_ipa.txt', 'r') as f_in, open('other_ipa2.txt', 'w') as f_out:
    for i, line in enumerate(f_in):
        if line.endswith('\n'):
            f_out.write(f"{line.rstrip()}\t{i}\n")  # Handles lines with \n
        else:
            f_out.write(f"{line}\t{i}")  # Handles last line (if no \n)