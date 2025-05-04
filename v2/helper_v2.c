#include "structures_v2.h"
#include <stdbool.h>

bool check_coord_validity(solver_v2_t* solver, coords_v2_t coords) {
	if (coords.x < 0 || coords.y < 0)
		return false;
	if (coords.y >= solver->space.height || coords.x >= solver->space.width)
		return false;

	return true;
}

cell_v2_t* get_cell_by_coords(solver_v2_t* solver, coords_v2_t coords) {
	if (!check_coord_validity(solver, coords))
		return 0;

	return solver->space.cells[coords.x][coords.y][coords.z];
}