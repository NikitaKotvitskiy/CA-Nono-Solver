import json
from nonogram import Nonogram

def load_nonogram_from_json(path: str) -> Nonogram:
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    width = data["width"]
    height = data["height"]
    n = Nonogram(width, height)

    n.row_hints = data.get("horizontal_blocks_lengths", [[] for _ in range(height)])
    n.col_hints = data.get("vertical_blocks_lengths", [[] for _ in range(width)])

    return n

def save_nonogram_to_json(nonogram: Nonogram, path: str):
    nonogram.generate_hints_from_grid()

    data = {
        "width": nonogram.width,
        "height": nonogram.height,
        "horizontal_blocks_lengths": nonogram.row_hints,
        "vertical_blocks_lengths": nonogram.col_hints
    }

    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=4)
