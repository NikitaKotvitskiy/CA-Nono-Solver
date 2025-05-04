#pragma once

#include "structures_v2.h"
#include <stdbool.h>

#define MAX_STEPS_WITHOUT_PROGRESS 20
#define CELL_GRAVITY_POWER_INCREASE_COEFFICIENT 1.3
#define BLOCK_GRAVITY_POWER_INCREASE_COEFFICIENT 1.3

int solve(solver_v2_t* solver, int max_iteration_count, bool debug);