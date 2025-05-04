import os
import json
import math
import random
from datetime import datetime

def generate_matrix(rows, cols, num_filled):
    """
    Generates a matrix of size rows x cols (False = empty, True = filled),
    filling exactly num_filled cells at random.
    """
    total_cells = rows * cols
    matrix = [[False] * cols for _ in range(rows)]
    indices = [(i, j) for i in range(rows) for j in range(cols)]
    random.shuffle(indices)

    # Fill the first num_filled cells in the shuffled list
    for i, j in indices[:num_filled]:
        matrix[i][j] = True
    return matrix

def get_row_clues(matrix):
    """
    Calculates the consecutive filled block lengths for each row.
    Example: [True, True, False, True] -> blocks = [2, 1].
    """
    clues = []
    for row in matrix:
        row_clues = []
        count = 0
        for cell in row:
            if cell:
                count += 1
            else:
                if count:
                    row_clues.append(count)
                    count = 0
        # End of row: if it ended with a filled block, add it
        if count:
            row_clues.append(count)
        clues.append(row_clues)
    return clues

def get_column_clues(matrix):
    """
    Calculates the consecutive filled block lengths for each column.
    """
    clues = []
    rows = len(matrix)
    cols = len(matrix[0]) if rows > 0 else 0

    for col in range(cols):
        col_clues = []
        count = 0
        for row in range(rows):
            if matrix[row][col]:
                count += 1
            else:
                if count:
                    col_clues.append(count)
                    count = 0
        # End of column: if it ended with a filled block, add it
        if count:
            col_clues.append(count)
        clues.append(col_clues)
    return clues

def get_all_block_lengths(row_clues, col_clues):
    """
    Merges all row and column block lengths into a single list.
    """
    all_lengths = []
    for rc in row_clues:
        all_lengths.extend(rc)
    for cc in col_clues:
        all_lengths.extend(cc)
    return all_lengths

def median(values):
    """
    Returns the median of a list of numbers.
    If the list is empty, returns 0.
    """
    if not values:
        return 0
    s = sorted(values)
    n = len(s)
    mid = n // 2
    if n % 2 == 1:
        return s[mid]
    else:
        return (s[mid - 1] + s[mid]) / 2

def main():
    # User inputs
    N = int(input("Enter the number of configurations (N): "))
    X = int(input("Enter the width (X): "))
    Y = int(input("Enter the height (Y): "))
    A = float(input("Enter the minimal fill percentage (e.g. 1): "))
    B = float(input("Enter the maximum fill percentage (e.g. 99): "))

    # Convert percentages to fractions
    A_frac = A / 100.0
    B_frac = B / 100.0

    # Generate a list of fill fractions evenly spaced from A_frac to B_frac
    if N > 1:
        fill_fractions = [
            A_frac + i * (B_frac - A_frac) / (N - 1)
            for i in range(N)
        ]
    else:
        fill_fractions = [A_frac]

    total_cells = X * Y
    # Calculate the minimal and maximal number of filled cells
    min_filled = math.ceil(total_cells * A_frac)
    max_filled = math.floor(total_cells * B_frac)
    if min_filled < 0:
        min_filled = 0
    if max_filled > total_cells:
        max_filled = total_cells

    # Create folder name
    folder_name = f"{X}x{Y}"
    os.makedirs(folder_name, exist_ok=True)

    info_lines = []
    list_lines = []

    for idx, fill in enumerate(fill_fractions):
        # Desired number of filled cells before clamping
        desired_filled = round(total_cells * fill)
        # Clamp to avoid going below A or above B
        if desired_filled < min_filled:
            desired_filled = min_filled
        if desired_filled > max_filled:
            desired_filled = max_filled

        # Generate matrix
        matrix = generate_matrix(Y, X, desired_filled)

        # Calculate the actual fill fraction
        actual_filled = sum(1 for row in matrix for cell in row if cell)
        actual_fill_fraction = actual_filled / total_cells

        # Clues
        row_clues = get_row_clues(matrix)
        col_clues = get_column_clues(matrix)

        # Build JSON config
        config = {
            "width": X,
            "height": Y,
            "vertical_blocks_lengths": col_clues,
            "horizontal_blocks_lengths": row_clues
        }

        # Save JSON file
        file_name = f"{idx+1}.json"
        file_path = os.path.join(folder_name, file_name)
        with open(file_path, "w", encoding="utf-8") as f:
            json.dump(config, f, indent=4, ensure_ascii=False)

        # Calculate the median block length (rows + columns)
        all_blocks = get_all_block_lengths(row_clues, col_clues)
        med_value = median(all_blocks)

        # Prepare info line
        info_lines.append(
            f"{file_name}: fill = {round(actual_fill_fraction * 100, 2)}%, "
            f"median block length = {med_value}"
        )

        # Collect file name for list.txt
        list_lines.append(file_name)

    # Write info.txt in UTF-8
    info_path = os.path.join(folder_name, "info.txt")
    with open(info_path, "w", encoding="utf-8") as f:
        f.write("\n".join(info_lines))

    # Write list.txt with the names of generated files
    list_path = os.path.join(folder_name, "list.txt")
    with open(list_path, "w", encoding="utf-8") as f:
        f.write("\n".join(list_lines))

    print(f"Generated {N} configurations in folder: {folder_name}")

if __name__ == "__main__":
    main()
