#include "evaluator.h"
#include "structures.h"
#include <stdlib.h>

char check_border(nono_solver_t* solver, int x, int y) {
	if (x < 0 || x >= solver->space->width || y < 0 || y >= solver->space->height)
		return 0;
	return 1;
}

void applicate_rule_1(nono_solver_t* solver, ca_cell_t* cell, int opposite_z_coord) {
	if (solver->space->cells[cell->x][cell->y][opposite_z_coord]->type == colored)
		cell->colored_cell.evaluation += 2;
}

void applicate_rule_2(nono_solver_t* solver, ca_cell_t* cell, int opposite_z_coord) {
	if (solver->space->cells[cell->x][cell->y][opposite_z_coord]->type != colored)
		cell->colored_cell.evaluation -= 1;
}

void applicate_rule_3(nono_solver_t* solver, ca_cell_t* cell, int opposite_z_coord) {
	if (cell->colored_cell.is_border_cell == 1
		&& cell->colored_cell.border_direction != 0
		&& solver->space->cells[cell->x][cell->y][opposite_z_coord]->type == colored)
		cell->colored_cell.evaluation += 1;
}

void applicate_rule_4(nono_solver_t* solver, ca_cell_t* cell, int opposite_z_coord) {
	if (cell->colored_cell.is_border_cell == 1
		&& cell->colored_cell.border_direction == 0
		&& solver->space->cells[cell->x][cell->y][opposite_z_coord]->type == colored)
		cell->colored_cell.evaluation += 2;
}

void applicate_rule_5(nono_solver_t* solver, ca_cell_t* cell, int opposite_z_coord) {
	if (cell->colored_cell.block->colored_cells_count > 1) {
		if (cell->colored_cell.is_border_cell == 1) {
			int neighbour_cell_index = cell->colored_cell.blockIndex - cell->colored_cell.border_direction;
			ca_cell_t* neighbour_cell = cell->colored_cell.block->colored_cells[neighbour_cell_index];
			if (solver->space->cells[neighbour_cell->x][neighbour_cell->y][opposite_z_coord]->type == colored)
				cell->colored_cell.evaluation += 1;
		}
		else {
			int neighbour_1_cell_index = cell->colored_cell.blockIndex - 1;
			int neighbour_2_cell_index = cell->colored_cell.blockIndex + 1;
			ca_cell_t* neighbour_1_cell = cell->colored_cell.block->colored_cells[neighbour_1_cell_index];
			ca_cell_t* neighbour_2_cell = cell->colored_cell.block->colored_cells[neighbour_2_cell_index];
			if (solver->space->cells[neighbour_1_cell->x][neighbour_1_cell->y][opposite_z_coord]->type == colored)
				cell->colored_cell.evaluation += 1;
			if (solver->space->cells[neighbour_2_cell->x][neighbour_2_cell->y][opposite_z_coord]->type == colored)
				cell->colored_cell.evaluation += 1;
		}
	}
}

void applicate_rule_6(nono_solver_t* solver, ca_cell_t* cell) {
	if (cell->colored_cell.is_border_cell) {
		if (cell->colored_cell.border_direction != 0) {
			int x = cell->x + (cell->z == 0 ? 0 : cell->colored_cell.border_direction);
			int y = cell->y + (cell->z == 1 ? cell->colored_cell.border_direction : 0);
			if (check_border(solver, x, y)) {
				ca_cell_t* directed_cell = solver->space->cells[x][y][cell->z];
				if (directed_cell->type == empty
					&& directed_cell->empty_cell.pressure != 0
					&& directed_cell->empty_cell.pressure->direction != cell->colored_cell.border_direction)
					cell->colored_cell.evaluation -= 4 * cell->colored_cell.block->colored_cells_count;
			}
		}
		else {
			int x1, y1, x2, y2;
			x1 = cell->x + (cell->z == 0 ? 0 : 1);
			y1 = cell->y + (cell->z == 0 ? 1 : 0);
			x2 = cell->x + (cell->z == 0 ? 0 : -1);
			y2 = cell->y + (cell->z == 0 ? -1 : 0);

			if (check_border(solver, x1, y1)) {
				ca_cell_t* directed_cell_1 = solver->space->cells[x1][y1][cell->z];
				if (directed_cell_1->type == empty
					&& directed_cell_1->empty_cell.pressure != 0
					&& directed_cell_1->empty_cell.pressure->direction != 1)
					cell->colored_cell.evaluation -= 4 * cell->colored_cell.block->colored_cells_count;
			}

			if (check_border(solver, x2, y2)) {
				ca_cell_t* directed_cell_2 = solver->space->cells[x2][y2][cell->z];
				if (directed_cell_2->type == empty
					&& directed_cell_2->empty_cell.pressure != 0
					&& directed_cell_2->empty_cell.pressure->direction != -1)
					cell->colored_cell.evaluation -= 4 * cell->colored_cell.block->colored_cells_count;
			}
		}
	}
}

void applicate_rule_7(nono_solver_t* solver, ca_cell_t* cell) {
	int line_index = cell->z == 0 ? cell->y : solver->space->height + cell->x;
	int colored_cells_missing = solver->lines_list[line_index]->required_colored_cells_count - solver->lines_list[line_index]->actual_colored_cells_count;
	if (colored_cells_missing > 0)
		cell->colored_cell.evaluation += 1 * colored_cells_missing;
}

void applicate_rule_8(nono_solver_t* solver, ca_cell_t* cell) {
	int line_index = cell->z == 0 ? cell->y : solver->space->height + cell->x;
	int colored_cells_extra = solver->lines_list[line_index]->actual_colored_cells_count - solver->lines_list[line_index]->required_colored_cells_count;
	if (colored_cells_extra > 0)
		cell->colored_cell.evaluation -= 1 * colored_cells_extra;
}

void applicate_rule_9(nono_solver_t* solver, ca_cell_t* cell, int opposite_z_coord) {
	if (cell->colored_cell.is_border_cell == 1) {
		ca_cell_t** directed_cells = (ca_cell_t**)calloc(2, sizeof(ca_cell_t*));
		if (directed_cells == 0)
			exit(1);

		if (cell->colored_cell.border_direction == 0) {
			int x1, y1, x2, y2;
			x1 = cell->x + (cell->z == 1 ? 1 : 0);
			y1 = cell->y + (cell->z == 0 ? 1 : 0);
			if (check_border(solver, x1, y1))
				directed_cells[0] = solver->space->cells[x1][y1][opposite_z_coord];

			x2 = cell->x + (cell->z == 1 ? -1 : 0);
			y2 = cell->y + (cell->z == 0 ? -1 : 0);
			if (check_border(solver, x2, y2))
				directed_cells[1] = solver->space->cells[x2][y2][opposite_z_coord];
		}
		else {
			int x, y;
			x = cell->x + (cell->z == 1 ? 1 * cell->colored_cell.border_direction : 0);
			y = cell->y + (cell->z == 0 ? 1 * cell->colored_cell.border_direction : 0);
			if (check_border(solver, x, y))
				directed_cells[0] = solver->space->cells[x][y][opposite_z_coord];
		}

		for (int i = 0; i < 2; i++)
			if (directed_cells[i] != 0 && directed_cells[i]->type == colored)
				cell->colored_cell.evaluation -= 2;
		free(directed_cells);
	}
}

void applicate_rule_10(nono_solver_t* solver, ca_cell_t* cell, int opposite_z_coord) {
	ca_cell_t** orto_cells = (ca_cell_t**)calloc(2, sizeof(ca_cell_t*));
	if (orto_cells == 0)
		exit(1);

	int x1, x2, y1, y2;
	x1 = cell->x + (cell->z == 0 ? 1 : 0);
	y1 = cell->y + (cell->z == 1 ? 1 : 0);
	x2 = cell->x + (cell->z == 0 ? -1 : 0);
	y2 = cell->y + (cell->z == 1 ? -1 : 0);

	if (check_border(solver, x1, y1)) {
		ca_cell_t* orto_cell = solver->space->cells[x1][y1][opposite_z_coord];
		if (orto_cell->type == colored
			&& orto_cell->colored_cell.is_border_cell
			&& orto_cell->colored_cell.border_direction != 1)
			orto_cells[0] = orto_cell;
	}

	if (check_border(solver, x2, y2)) {
		ca_cell_t* orto_cell = solver->space->cells[x2][y2][opposite_z_coord];
		if (orto_cell->type == colored
			&& orto_cell->colored_cell.is_border_cell
			&& orto_cell->colored_cell.border_direction != -1)
			orto_cells[1] = orto_cell;
	}

	for (int i = 0; i < 2; i++)
		if (orto_cells[i] != 0)
			cell->colored_cell.evaluation -= 1;
	free(orto_cells);
}

int evaluate_cell(nono_solver_t* solver, ca_cell_t* cell) {
	cell->colored_cell.evaluation = 0;
	int opposite_z_coord = (cell->z + 1) % 2;

	applicate_rule_1(solver, cell, opposite_z_coord);
	applicate_rule_2(solver, cell, opposite_z_coord);
	applicate_rule_3(solver, cell, opposite_z_coord);
	applicate_rule_4(solver, cell, opposite_z_coord);
	applicate_rule_5(solver, cell, opposite_z_coord);
	applicate_rule_6(solver, cell);
	applicate_rule_7(solver, cell);
	applicate_rule_8(solver, cell);
	applicate_rule_9(solver, cell, opposite_z_coord);
	applicate_rule_10(solver, cell, opposite_z_coord);

	return cell->colored_cell.evaluation;
}

void count_colored_cells_in_lines(nono_solver_t* solver) {
	for (int i = 0; i < solver->space->width + solver->space->height; i++) {
		line_t* line = solver->lines_list[i];
		line->actual_colored_cells_count = 0;
		if (i < solver->space->height) {
			int y = i;
			for (int x = 0; x < solver->space->width; x++) {
				ca_cell_t* cell = solver->space->cells[x][y][0];
				if (cell->type == colored)
					line->actual_colored_cells_count++;
			}
		}
		else {
			int x = i - solver->space->height;
			for (int y = 0; y < solver->space->height; y++) {
				ca_cell_t* cell = solver->space->cells[x][y][1];
				if (cell->type == colored)
					line->actual_colored_cells_count++;
			}
		}
	}
}

void evaluate_ca(nono_solver_t* solver) {
	for (int i = 0; i < 2; i++) {
		plane_t* plane = solver->planes_list[i];
		double plane_evaluation_sum = 0;

		for (int j = 0; j < plane->blocks_count; j++) {
			block_t* block = plane->blocks_list[j];
			double block_evaluation_sum = 0;

			for (int z = 0; z < block->colored_cells_count; z++) {
				ca_cell_t* cell = block->colored_cells[z];
				int cell_evaluation = evaluate_cell(solver, cell);
				block_evaluation_sum += cell_evaluation;
			}

			block->evaluation = (double)block_evaluation_sum / block->colored_cells_count;
			plane_evaluation_sum += block->evaluation;
		}

		plane->evaluation = (double)plane_evaluation_sum / plane->blocks_count;
	}
}