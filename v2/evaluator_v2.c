#include "structures_v2.h"
#include "evaluator_v2.h"

#include <math.h>

double calculate_force_v2(coords_v2_t a, coords_v2_t b) {
	double dx = (double)a.x - (double)b.x;
	double dy = (double)a.y - (double)b.y;
	double dz = (double)a.z - (double)b.z;

	double distance = sqrt(dx * dx + dy * dy + dz * dz);
	return 1.0 / distance;
}

int get_direction_v2(coords_v2_t affected, coords_v2_t affecting) {
	int affected_coord = affected.z ? affected.x : affected.y;
	int affecting_coord = affected.z ? affecting.x : affecting.y;
	if (affecting_coord > affected_coord)
		return 1;
	if (affecting_coord < affected_coord)
		return -1;
	return 0;
}

void evaluate_cell_v2(solver_v2_t* solver, cell_v2_t* cell) {
	cell->colored.force_to_stay = 0;
	cell->colored.force_to_move = 0;
	int plane = cell->coords.z;

	int start_block_index = plane == 0 ? solver->space.vertical_blocks_count : 0;
	int end_block_index = plane == 0 ? solver->space.vertical_blocks_count + solver->space.horizontal_blocks_count - 1 : solver->space.vertical_blocks_count - 1;
	int fixed_coord = plane == 0 ? cell->coords.x : cell->coords.y;

	for (int i = start_block_index; i <= end_block_index; i++) {
		block_v2_t* block = solver->space.blocks[i];

		for (int j = 0; j < block->cells_count; j++) {
			cell_v2_t* op_cell = block->cells[j];
			int op_fixed_coord = plane == 0 ? op_cell->coords.y : op_cell->coords.x;
			
			if (fixed_coord < op_cell->colored.min || fixed_coord > op_cell->colored.max ||
				op_fixed_coord < cell->colored.min || op_fixed_coord > cell->colored.max)
				continue;

			double force = calculate_force_v2(cell->coords, op_cell->coords) * op_cell->colored.gravity_power;
			int direction = get_direction_v2(cell->coords, op_cell->coords);
			if (direction == 0)
				cell->colored.force_to_stay += force;
			else
				cell->colored.force_to_move += force * direction;
		}
	}
}

void evaluate_block_v2(solver_v2_t* solver, block_v2_t* block) {
	if (block->max_possible_coord == block->min_possible_coord)
		return;

	block->force_to_move = 0;
	block->force_to_stay = 0;

	for (int i = 0; i < block->cells_count; i++) {
		cell_v2_t* cell = block->cells[i];
		evaluate_cell_v2(solver, cell);
		block->force_to_move += cell->colored.force_to_move;
		block->force_to_stay += cell->colored.force_to_stay;
	}

	block->force_to_move /= block->cells_count;
	block->force_to_stay /= block->cells_count;

	int plane = block->cells[0]->coords.z;
	if (plane == 0) {
		for (int y = 0; y < block->cells[0]->coords.y - 1; y++) {
			cell_v2_t* cell = solver->space.cells[block->cells[0]->coords.x][y][plane];
			if (cell->type == colored) {
				coords_v2_t border_coords = cell->colored.block->cells[cell->colored.block->cells_count - 1]->coords;
				block->force_to_move += calculate_force_v2(border_coords, block->cells[0]->coords) * cell->colored.block->gravity_power;
				y = border_coords.y + 1;
			}
		}
		for (int y = block->cells[block->cells_count - 1]->coords.y + 2; y < solver->space.height; y++) {
			cell_v2_t* cell = solver->space.cells[block->cells[0]->coords.x][y][plane];
			if (cell->type == colored) {
				coords_v2_t border_coords = cell->coords;
				block->force_to_move -= calculate_force_v2(border_coords, block->cells[block->cells_count - 1]->coords) * cell->colored.block->gravity_power;
				y = cell->colored.block->cells[cell->colored.block->cells_count - 1]->coords.y + 1;
			}
		}
	}
	else {
		for (int x = 0; x < block->cells[block->cells_count - 1]->coords.x - 1; x++) {
			cell_v2_t* cell = solver->space.cells[x][block->cells[0]->coords.y][plane];
			if (cell->type == colored) {
				coords_v2_t border_coords = cell->colored.block->cells[0]->coords;
				block->force_to_move += calculate_force_v2(border_coords, block->cells[block->cells_count - 1]->coords) * cell->colored.block->gravity_power;
				x = cell->colored.block->cells[0]->coords.x + 1;
			}
		}
		for (int x = block->cells[0]->coords.x + 1; x < solver->space.width; x++) {
			cell_v2_t* cell = solver->space.cells[x][block->cells[0]->coords.y][plane];
			if (cell->type == colored) {
				coords_v2_t border_coords = cell->coords;
				block->force_to_move -= calculate_force_v2(border_coords, block->cells[0]->coords) * cell->colored.block->gravity_power;
				x = cell->colored.block->cells[0]->coords.x + 1;
			}
		}
	}
}

void evaluate_all_v2(solver_v2_t* solver) {
	for (int i = 0; i < solver->space.vertical_blocks_count + solver->space.horizontal_blocks_count; i++) {
		evaluate_block_v2(solver, solver->space.blocks[i]);
	}
}