import random
import string

def generate_input_file(filename, check_file, row_num, row_size, h_weight=5):
    letters = string.ascii_uppercase
    weights = [h_weight if c == 'H' else 1 for c in letters]  # H is more frequent

    with open(filename, 'w') as f:
        with open(check_file, 'w') as f_c:
            for _ in range(row_num):
                line = ''.join(random.choices(letters, weights=weights, k=row_size))
                h_count = line.count('H')
                f_c.write(str(h_count) + '\n')
                f.write(line + '\n')


print("Generating 1...")

# Parameters - Challenge 1
row_num = 100000            # Number of lines in the file
row_size = 256              # Number of random letters per line
h_weight = 5                # How much more frequent 'H' should be


generate_input_file('input_c1.txt', 'check_c1.txt', row_num, row_size, h_weight)

print("Generating 2...")

# Parameters - Challenge 2
row_num = 256               # Number of lines in the file
row_size = 100000           # Number of random letters per line
h_weight = 5                # How much more frequent 'H' should be

# Generate the file
generate_input_file('input_c2.txt', 'check_c2.txt', row_num, row_size, h_weight)

