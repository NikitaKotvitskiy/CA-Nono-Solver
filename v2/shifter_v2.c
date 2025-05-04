#include "structures_v2.h"
#include "shifter_v2.h"
#include "helper_v2.h"

#include <stdbool.h>

block_v2_t* can_block_be_placed_at(solver_v2_t* solver, block_v2_t* block, int pos) {
	int plane = block->cells[0]->coords.z;
	if (pos < block->min_possible_coord || pos > block->max_possible_coord)
		return (block_v2_t *)-1;

	int start_pos = pos - 1;
	int end_pos = pos + block->cells_count;

	coords_v2_t coords = block->cells[plane == 0 ? 0 : block->cells_count - 1]->coords;
	for (int i = start_pos; i <= end_pos; i++) {
		if (plane == 0)
			coords.y = i;
		else
			coords.x = i;

		cell_v2_t* cell = get_cell_by_coords(solver, coords);
		if (cell == 0)
			continue;

		if (cell->type == colored && cell->colored.block != block)
			return cell->colored.block;
	}

	return 0;
}

void swap_cells(solver_v2_t* solver, cell_v2_t* cell, coords_v2_t coords) {
	cell_v2_t* cell2 = get_cell_by_coords(solver, coords);
	solver->space.cells[coords.x][coords.y][coords.z] = cell;
	solver->space.cells[cell->coords.x][cell->coords.y][cell->coords.z] = cell2;
	cell2->coords = cell->coords;
	cell->coords = coords;
}

void move_block_at(solver_v2_t* solver, block_v2_t* block, int pos) {
	int plane = block->cells[0]->coords.z;
	coords_v2_t target_coords = block->cells[0]->coords;
	if (plane == 0)
		for (int i = 0; i < block->cells_count; i++) {
			target_coords.y = pos + i;
			swap_cells(solver, block->cells[i], target_coords);
		}
	else
		for (int i = block->cells_count - 1; i >= 0; i--) {
			target_coords.x = pos + block->cells_count - 1 - i;
			swap_cells(solver, block->cells[i], target_coords);
		}
}