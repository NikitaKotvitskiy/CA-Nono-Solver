import subprocess
import os
import json

def run_solver_with_file(input_path: str, solver_path: str) -> tuple[bool, list[list[int]] | None, str]:
    output_path = input_path + ".out.json"

    try:
        args = [solver_path, "-solve", input_path, output_path]
        result = subprocess.run(args, capture_output=True, text=True)

        if result.returncode != 0:
            return False, None, f"Solver failed:\n{result.stderr.strip()}"

        if not os.path.exists(output_path):
            return False, None, "Solver did not produce output file."

        with open(output_path, "r", encoding="utf-8") as f:
            data = json.load(f)

        if not data.get("solved", False):
            return False, None, "Solver could not find a solution."

        solution = data.get("solution")
        if not solution:
            return False, None, "No solution found in output file."

        return True, solution, "Solution found successfully."

    except Exception as e:
        return False, None, f"Solver error:\n{str(e)}"
