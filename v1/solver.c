#include "solver.h"
#include "structures.h"
#include "printer.h"
#include "evaluator.h"
#include "loop-detector.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <stdbool.h>

void empty_cells_control(nono_solver_t* solver) {
	for (int x = 0; x < solver->space->width; x++)
		for (int y = 0; y < solver->space->height; y++)
			for (int z = 0; z < 2; z++) {
				ca_cell_t* cell = solver->space->cells[x][y][z];
				if (cell->type != empty)
					continue;

				if (cell->empty_cell.shift != NULL) {
					cell->empty_cell.shift->block->shift = NULL;
					free(cell->empty_cell.shift);
					cell->empty_cell.shift = NULL;
				}

				if (cell->empty_cell.pressure != NULL) {
					int neigh_1_x = z == 1 ? x + 1 : x;
					int neigh_1_y = z == 0 ? y + 1 : y;
					int neigh_2_x = z == 1 ? x - 1 : x;
					int neigh_2_y = z == 0 ? y - 1 : y;
					int colored_neighbours_count = 0;
					if (check_border(solver, neigh_1_x, neigh_1_y)
						&& solver->space->cells[neigh_1_x][neigh_1_y][z]->type == colored)
						colored_neighbours_count++;
					if (check_border(solver, neigh_2_x, neigh_2_y)
						&& solver->space->cells[neigh_2_x][neigh_2_y][z]->type == colored)
						colored_neighbours_count++;
					if (colored_neighbours_count != 2) {
						free(cell->empty_cell.pressure);
						cell->empty_cell.pressure = NULL;
					}
				}
			}
}

int define_move_directions(nono_solver_t* solver, char plane) {
	double average_evaluation = (solver->planes_list[0]->evaluation + solver->planes_list[1]->evaluation) / 2;

	int start_block_index = plane == 0 ? 0 : solver->planes_list[0]->blocks_count;
	int end_block_index = plane == 0 ? solver->planes_list[0]->blocks_count : solver->planes_list[0]->blocks_count + solver->planes_list[1]->blocks_count;
	for (int i = start_block_index; i < end_block_index; i++) {
		block_t* block = solver->blocks_list[i];
		if (block->evaluation > average_evaluation)
			continue;

		char can_move_forward = 1;
		char can_move_back = 1;

		ca_cell_t* border_cell_1 = block->colored_cells[0];
		ca_cell_t* border_cell_2 = block->colored_cells[block->colored_cells_count - 1];

		int x1 = border_cell_1->x + (border_cell_1->z == 1 ? -1 : 0);
		int y1 = border_cell_1->y + (border_cell_1->z == 0 ? -1 : 0);

		if (!check_border(solver, x1, y1))
			can_move_back = 0;

		int x2 = border_cell_2->x + (border_cell_2->z == 1 ? 1 : 0);
		int y2 = border_cell_2->y + (border_cell_2->z == 0 ? 1 : 0);

		if (!check_border(solver, x2, y2))
			can_move_forward = 0;

		if (!can_move_back && !can_move_forward)
			continue;

		if (can_move_back && can_move_forward) {
			can_move_forward = block->last_movement_direction == 1 ? 1 : 0;
			can_move_back = block->last_movement_direction == -1 ? 1 : 0;
		}

		block->shift = malloc(sizeof(shift_alert_t));
		if (block->shift == NULL) {
			fprintf(stderr, "Memory allocation error\n");
			return -1;
		}
		block->shift->evaluation = block->evaluation;
		block->shift->direction = can_move_back ? -1 : 1;
		block->shift->block = block;

		int x = can_move_back ? x1 : x2;
		int y = can_move_back ? y1 : y2;
		ca_cell_t* cell_in_move_direction = solver->space->cells[x][y][border_cell_1->z];

		if (cell_in_move_direction->empty_cell.shift == NULL)
			cell_in_move_direction->empty_cell.shift = block->shift;
		else {
			shift_alert_t* concurrent_shift = cell_in_move_direction->empty_cell.shift;

			pressure_alert_t* pressure_alert = malloc(sizeof(pressure_alert_t));
			if (pressure_alert == NULL) {
				fprintf(stderr, "Memory allocation error\n");
				return -1;
			}

			if (block->shift->evaluation < concurrent_shift->evaluation)
				pressure_alert->direction = block->shift->direction;
			else if (block->shift->evaluation > concurrent_shift->evaluation)
				pressure_alert->direction = concurrent_shift->direction;
			else
				pressure_alert->direction = 0;

			free(block->shift);
			block->shift = NULL;
			concurrent_shift->block->shift = NULL;
			free(concurrent_shift);

			cell_in_move_direction->empty_cell.shift = NULL;
			cell_in_move_direction->empty_cell.pressure = pressure_alert;
		}
	}

	return 0;
}

int collisions_control(nono_solver_t* solver, char plane) {
	for (int x = 0; x < solver->space->width; x++)
		for (int y = 0; y < solver->space->height; y++) {

			ca_cell_t* cell = solver->space->cells[x][y][plane];
			if (cell->type != empty || cell->empty_cell.shift == NULL)
				continue;

			int directed_cell_x = x + (plane == 1 ? 1 : 0) * cell->empty_cell.shift->direction;
			int directed_cell_y = y + (plane == 0 ? 1 : 0) * cell->empty_cell.shift->direction;
			if (!check_border(solver, directed_cell_x, directed_cell_y))
				continue;
			ca_cell_t* directed_cell = solver->space->cells[directed_cell_x][directed_cell_y][plane];

			if (directed_cell->type == colored) {
				pressure_alert_t* pressure = malloc(sizeof(pressure_alert_t));
				if (pressure == NULL) {
					fprintf(stderr, "Memory allocation error\n");
					return -1;
				}

				pressure->direction = cell->empty_cell.shift->direction;
				cell->empty_cell.shift->block->shift = NULL;
				free(cell->empty_cell.shift);
				cell->empty_cell.shift = NULL;
				cell->empty_cell.pressure = pressure;
			}
			else if (directed_cell->empty_cell.shift != NULL) {
				double cell_evaluation = cell->empty_cell.shift->evaluation;
				double directed_cell_evaluation = directed_cell->empty_cell.shift->evaluation;

				if (cell_evaluation >= directed_cell_evaluation) {
					cell->empty_cell.shift->block->shift = NULL;
					free(cell->empty_cell.shift);
					cell->empty_cell.shift = NULL;
				}

				if (directed_cell_evaluation >= cell_evaluation) {
					directed_cell->empty_cell.shift->block->shift = NULL;
					free(directed_cell->empty_cell.shift);
					directed_cell->empty_cell.shift = NULL;
				}
			}
		}

	return 0;
}

void move_block(nono_solver_t* solver, block_t* block) {
	char plain_index = block->colored_cells[0]->z;

	int start = block->shift->direction == 1 ? block->colored_cells_count - 1 : 0;
	int end = block->shift->direction == -1 ? block->colored_cells_count - 1 : 0;

	int shift_cell_x = block->colored_cells[start]->x + (plain_index == 1 ? 1 : 0) * block->shift->direction;
	int shift_cell_y = block->colored_cells[start]->y + (plain_index == 0 ? 1 : 0) * block->shift->direction;
	ca_cell_t* tmp_cell = solver->space->cells[shift_cell_x][shift_cell_y][plain_index];
	tmp_cell->x = block->colored_cells[end]->x;
	tmp_cell->y = block->colored_cells[end]->y;

	end -= block->shift->direction;
	int index = start;
	while (index != end) {
		ca_cell_t* cell = block->colored_cells[index];
		cell->x += (plain_index == 1 ? 1 : 0) * block->shift->direction;
		cell->y += (plain_index == 0 ? 1 : 0) * block->shift->direction;
		solver->space->cells[cell->x][cell->y][plain_index] = cell;
		index -= block->shift->direction;
	}

	solver->space->cells[tmp_cell->x][tmp_cell->y][plain_index] = tmp_cell;
	block->last_movement_direction = block->shift->direction;
}

void block_shift(nono_solver_t* solver, char plane) {
	block_t** blocks_list = solver->planes_list[plane]->blocks_list;
	for (int i = 0; i < solver->planes_list[plane]->blocks_count; i++) {
		block_t* block = blocks_list[i];
		if (block->shift == NULL)
			continue;

		move_block(solver, block);
	}
}

int check_if_nono_is_solved(nono_solver_t* solver) {
	for (int i = 0; i < solver->planes_list[0]->blocks_count; i++) {
		block_t* block = solver->blocks_list[i];
		for (int j = 0; j < block->colored_cells_count; j++) {
			ca_cell_t* cell = block->colored_cells[j];
			ca_cell_t* op_cell = solver->space->cells[cell->x][cell->y][cell->z ? 0 : 1];
			if (op_cell->type == empty)
				return 0;
		}
	}

	return 1;
}

bool solve_nono(nono_solver_t* solver, bool debug) {
	int plane = 0;
	while (1) {
		if (check_if_nono_is_solved(solver)) {
			if (debug) printf("Nonogram is solved!\n");
			reset_loop_detection();
			return true;
		}

		empty_cells_control(solver);
		count_colored_cells_in_lines(solver);
		evaluate_ca(solver);
		if (debug) print_nonogram(solver);
		if (debug) print_plane_evaluations(solver);

		char check_loop_result = check_loop(solver, plane);
		if (check_loop_result == -1) {
			fprintf(stderr, "Memory allocation error\n");
			reset_loop_detection();
			return false;
		}
		else if (check_loop_result == 1) {
			if (debug) fprintf(stdout, "Loop detected\n");
			break;
		}

		if (define_move_directions(solver, plane) != 0) {
			reset_loop_detection();
			return false;
		}
		if (collisions_control(solver, plane) != 0) {
			reset_loop_detection();
			return false;
		}
		block_shift(solver, plane);

		plane = plane ? 0 : 1;
	}
	reset_loop_detection();
	return false;
}