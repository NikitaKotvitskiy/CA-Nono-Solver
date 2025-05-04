# CA-Nonogram-Solver

This repository contains an experimental nonogram solving toolkit based on **cellular automata**. It includes:

- Two different solver implementations (two different methods).
- A Python-based graphical interface (GUI) for creating and solving puzzles.
- A nonogram generator for testing and experimentation.

---

## Compiling and Running the Solvers

### Description

- `v1/` – **Living organisms method**: treats blocks as evolving entities with rules for interaction.
- `v2/` – **Magnetism method**: models the solution process with magnetic-like attraction of blocks.

You can compile both of them using **make** command, which generates two executable binaries: **method1** and **method2**.
You can launch each of them without arguments to get instructions about usage.

### Requirements

- GCC with C17 support
- GNU Make

---

## Running GUI

### Description

A Tkinter-based GUI that allows you to:

- Create nonograms by drawing them
- Load puzzles from files
- Solve nonograms manually or using automated methods

You can launch the GUI with command **python ./gui/main.py ./method1 ./method2**.

### Requirements

- Python 3
- Tkinter

---

## Nonogram generator

### Description

A command-line utility that creates random nonogram puzzles for testing.
You can launch the generator with command **python ./nonogen.py**.

### Requirements

- Python 3
