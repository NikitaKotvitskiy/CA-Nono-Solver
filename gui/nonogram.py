class Nonogram:
    def __init__(self, width: int, height: int):
        self.width = width
        self.height = height

        self.grid = [[2 for _ in range(width)] for _ in range(height)]
        self.row_hints = [[] for _ in range(height)]
        self.col_hints = [[] for _ in range(width)]

    def set_cell(self, x: int, y: int, value: int):
        if value in (0, 1, 2):
            self.grid[y][x] = value

    def get_cell(self, x: int, y: int) -> int:
        return self.grid[y][x]

    def clear_grid(self):
        self.grid = [[2 for _ in range(self.width)] for _ in range(self.height)]

    def generate_hints_from_grid(self):
        def extract_hints(line):
            hints = []
            count = 0
            for cell in line:
                if cell == 1:
                    count += 1
                elif count > 0:
                    hints.append(count)
                    count = 0
            if count > 0:
                hints.append(count)
            return hints or [0]

        self.row_hints = [extract_hints(row) for row in self.grid]
        self.col_hints = [
            extract_hints([self.grid[y][x] for y in range(self.height)])
            for x in range(self.width)
        ]

    def set_grid(self, grid_data):
        self.grid = grid_data
        self.height = len(grid_data)
        self.width = len(grid_data[0]) if self.height > 0 else 0

    def check_solution_matches_hints(self) -> bool:
        def extract_hints(line):
            hints = []
            count = 0
            for cell in line:
                if cell == 1:
                    count += 1
                elif count > 0:
                    hints.append(count)
                    count = 0
            if count > 0:
                hints.append(count)
            return hints or [0]

        for y in range(self.height):
            row = self.grid[y]
            if extract_hints(row) != self.row_hints[y]:
                return False

        for x in range(self.width):
            col = [self.grid[y][x] for y in range(self.height)]
            if extract_hints(col) != self.col_hints[x]:
                return False

        return True
