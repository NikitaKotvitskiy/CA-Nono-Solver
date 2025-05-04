import sys
import tkinter as tk
from ui import App

def main():
    if len(sys.argv) < 3:
        print("Usage: python main.py <path_to_solver1> <path_to_solver2>")
        sys.exit(1)

    solver1 = sys.argv[1]
    solver2 = sys.argv[2]

    root = tk.Tk()
    root.title("Nonogram editor")
    app = App(root, solver1, solver2)
    root.mainloop()

if __name__ == "__main__":
    main()
