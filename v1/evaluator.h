#pragma once
#include "structures.h"

char check_border(nono_solver_t* solver, int x, int y);
void evaluate_ca(nono_solver_t* solver);
void count_colored_cells_in_lines(nono_solver_t* solver);