import tkinter as tk

class NonogramWidget(tk.Canvas):
    def __init__(self, master, nonogram, editable=True, use_dynamic_hints=True, on_cell_click=None):
        self.nonogram = nonogram
        self.editable = editable
        self.use_dynamic_hints = use_dynamic_hints
        self.on_cell_click = on_cell_click

        super().__init__(master, bg="white")
        self.bind("<Button-1>", self.handle_click)
        self.bind("<Configure>", self.on_resize)

        self.current_width = 1
        self.current_height = 1
        self._cell_size = 1
        self._pad_x = 0
        self._pad_y = 0

    def on_resize(self, event):
        self.current_width = event.width
        self.current_height = event.height
        self.draw()

    def draw(self):
        self.delete("all")

        n = self.nonogram
        if self.use_dynamic_hints:
            n.generate_hints_from_grid()

        cols = n.width
        rows = n.height

        cell_width = max(1, (self.current_width - 60) // cols)
        cell_height = max(1, (self.current_height - 60) // rows)
        cell_size = min(cell_width, cell_height)

        grid_width_px = cols * cell_size
        grid_height_px = rows * cell_size

        hint_pad_x = 60
        hint_pad_y = 60

        total_width = grid_width_px + hint_pad_x
        total_height = grid_height_px + hint_pad_y

        offset_x = max(0, (self.current_width - total_width) // 2)
        offset_y = max(0, (self.current_height - total_height) // 2)

        self._pad_x = offset_x + hint_pad_x
        self._pad_y = offset_y + hint_pad_y

        for x in range(cols):
            hints = n.col_hints[x]
            for i, hint in enumerate(reversed(hints)):
                px = self._pad_x + x * cell_size + cell_size // 2
                py = self._pad_y - (i + 1) * 15
                self.create_text(px, py, text=str(hint), font=("Arial", 10))

        for y in range(rows):
            hints = n.row_hints[y]
            for i, hint in enumerate(reversed(hints)):
                px = self._pad_x - (i + 1) * 15
                py = self._pad_y + y * cell_size + cell_size // 2
                self.create_text(px, py, text=str(hint), font=("Arial", 10))

        # Draw grid
        for y in range(rows):
            for x in range(cols):
                state = n.get_cell(x, y)
                px = self._pad_x + x * cell_size
                py = self._pad_y + y * cell_size

                color = "gray" if state == 2 else "white" if state == 0 else "black"

                self.create_rectangle(
                    px, py, px + cell_size, py + cell_size,
                    fill=color, outline="black"
                )

        self._cell_size = cell_size

    def handle_click(self, event):
        if not self.editable:
            return

        x = (event.x - self._pad_x) // self._cell_size
        y = (event.y - self._pad_y) // self._cell_size

        if 0 <= x < self.nonogram.width and 0 <= y < self.nonogram.height:
            current = self.nonogram.get_cell(x, y)
            new_value = (current + 1) % 3
            self.nonogram.set_cell(x, y, new_value)
            self.draw()

            if self.on_cell_click:
                self.on_cell_click(x, y, new_value)