#pragma once

#include "structures_v2.h"
#include <stdbool.h>

bool check_coord_validity(solver_v2_t* solver, coords_v2_t coords);
cell_v2_t* get_cell_by_coords(solver_v2_t* solver, coords_v2_t coords);