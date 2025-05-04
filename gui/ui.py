import tkinter as tk
from tkinter import filedialog, messagebox
from enum import Enum
from nonogram import Nonogram
from nonogram_widget import NonogramWidget
from solver_interface import run_solver_with_file

class AppMode(Enum):
    START = 0
    CREATE = 1
    SOLVE = 2

class App:
    def __init__(self, root, solver1_path, solver2_path):
        self.root = root
        self.solver1_path = solver1_path
        self.solver2_path = solver2_path

        self.mode = AppMode.START
        self.nonogram = None
        self.grid_widget = None

        self.content_frame = tk.Frame(self.root)
        self.content_frame.pack(fill=tk.BOTH, expand=True)

        self.show_start_menu()

    def clear_frame(self):
        for widget in self.content_frame.winfo_children():
            widget.destroy()

    def show_start_menu(self):
        self.mode = AppMode.START
        self.nonogram = None
        self.grid_widget = None
        self.clear_frame()

        label = tk.Label(self.content_frame, text="Welcome to Nonogram editor", font=("Arial", 16))
        label.pack(pady=20)

        btn_create = tk.Button(self.content_frame, text="Create", width=20, command=self.enter_create_mode)
        btn_create.pack(pady=10)

        btn_load = tk.Button(self.content_frame, text="Load", width=20, command=self.load_nonogram)
        btn_load.pack(pady=10)

    def enter_create_mode(self):
        self.mode = AppMode.CREATE
        self.nonogram = None
        self.clear_frame()

        label = tk.Label(self.content_frame, text="Enter grid size", font=("Arial", 14))
        label.pack(pady=10)

        size_frame = tk.Frame(self.content_frame)
        size_frame.pack()

        tk.Label(size_frame, text="Width:").grid(row=0, column=0, padx=5)
        tk.Label(size_frame, text="Height:").grid(row=1, column=0, padx=5)

        self.width_entry = tk.Entry(size_frame, width=5)
        self.width_entry.grid(row=0, column=1)

        self.height_entry = tk.Entry(size_frame, width=5)
        self.height_entry.grid(row=1, column=1)

        tk.Button(self.content_frame, text="OK", command=self.initialize_grid_from_input).pack(pady=10)
        tk.Button(self.content_frame, text="Back", command=self.show_start_menu).pack()

    def initialize_grid_from_input(self):
        try:
            width = int(self.width_entry.get())
            height = int(self.height_entry.get())
            if width <= 0 or height <= 0:
                raise ValueError
        except ValueError:
            messagebox.showerror("Invalid input", "Width and height must be positive integers.")
            return

        self.nonogram = Nonogram(width, height)
        self.show_editor()

    def show_editor(self):
        self.mode = AppMode.CREATE
        self.clear_frame()

        menu_frame = tk.Frame(self.content_frame)
        menu_frame.pack(pady=5)

        tk.Button(menu_frame, text="Create", command=self.enter_create_mode).pack(side=tk.LEFT, padx=5)
        tk.Button(menu_frame, text="Load", command=self.load_nonogram).pack(side=tk.LEFT, padx=5)
        tk.Button(menu_frame, text="Save", command=self.save_nonogram).pack(side=tk.LEFT, padx=5)

        self.grid_widget = NonogramWidget(self.content_frame, self.nonogram, editable=True)
        self.grid_widget.pack(expand=True, fill=tk.BOTH)

    def save_nonogram(self):
        if self.nonogram is None:
            return

        path = filedialog.asksaveasfilename(title="Save configuration", defaultextension=".json",
                                            filetypes=[("JSON files", "*.json")])
        if not path:
            return

        from parser import save_nonogram_to_json
        try:
            save_nonogram_to_json(self.nonogram, path)
            messagebox.showinfo("Saved", "Nonogram saved successfully.")
        except Exception as e:
            messagebox.showerror("Error", f"Could not save file:\n{e}")

    def load_nonogram(self):
        path = filedialog.askopenfilename(title="Open nonogram", filetypes=[("JSON files", "*.json")])
        if not path:
            return

        from parser import load_nonogram_from_json
        try:
            self.nonogram = load_nonogram_from_json(path)
            self.last_loaded_path = path

            self.nonogram.clear_grid()

            self.show_solver()
        except Exception as e:
            messagebox.showerror("Error", f"Could not load file:\n{e}")


    def show_solver(self):
        self.mode = AppMode.SOLVE
        self.clear_frame()

        menu_frame = tk.Frame(self.content_frame)
        menu_frame.pack(pady=5)

        tk.Button(menu_frame, text="Create", command=self.enter_create_mode).pack(side=tk.LEFT, padx=5)
        tk.Button(menu_frame, text="Load", command=self.load_nonogram).pack(side=tk.LEFT, padx=5)
        tk.Button(menu_frame, text="Solve (1)", command=self.solve_with_solver1).pack(side=tk.LEFT, padx=5)
        tk.Button(menu_frame, text="Solve (2)", command=self.solve_with_solver2).pack(side=tk.LEFT, padx=5)
        tk.Button(menu_frame, text="Check", command=self.check_user_solution).pack(side=tk.LEFT, padx=5)
        tk.Button(menu_frame, text="Reset grid", command=self.reset_grid).pack(side=tk.LEFT, padx=5)

        self.grid_widget = NonogramWidget(self.content_frame, self.nonogram, editable=True, use_dynamic_hints=False)
        self.grid_widget.pack(expand=True, fill=tk.BOTH)

    def check_user_solution(self):
        if self.nonogram.check_solution_matches_hints():
            messagebox.showinfo("Correct", "Your solution matches the hints!")
        else:
            messagebox.showwarning("Incorrect", "Your solution does not match the hints.")


    def solve_with_solver1(self):
        self.run_solver(self.solver1_path)

    def solve_with_solver2(self):
        self.run_solver(self.solver2_path)

    def run_solver(self, solver_path):
        if not hasattr(self, "last_loaded_path") or not self.last_loaded_path:
            messagebox.showerror("Error", "No file loaded to solve.")
            return

        success, grid, msg = run_solver_with_file(self.last_loaded_path, solver_path)
        if success:
            self.nonogram.set_grid(grid)
            self.grid_widget.nonogram = self.nonogram
            self.grid_widget.draw()
            messagebox.showinfo("Solved", msg)
        else:
            messagebox.showwarning("Unsolved", msg)



    def reset_grid(self):
        if not self.nonogram:
            return
        self.nonogram.clear_grid()
        self.grid_widget.draw()