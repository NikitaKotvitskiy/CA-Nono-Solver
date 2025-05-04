#include "printer.h"
#include "structures.h"
#include <stdio.h>

void print_nonogram(nono_solver_t* solver) {
	// Print plane with vertical blocks
	printf("    ");
	for (int i = 0; i < solver->space->width; i++)
		printf("-");
	printf("\n");
	for (int y = 0; y < solver->space->height; y++) {
		printf("%d/%d|", solver->lines_list[y]->actual_colored_cells_count, solver->lines_list[y]->required_colored_cells_count);
		for (int x = 0; x < solver->space->width; x++) {
			if (solver->space->cells[x][y][0]->type == empty)
				printf(" ");
			else
				printf("X");
		}
		printf("|\n");
	}
	printf("    ");
	for (int i = 0; i < solver->space->width; i++)
		printf("-");
	printf("\n");

	printf("\n ");
	// Print plane with horizontal blocks
	for (int i = 0; i < solver->space->width; i++)
		printf("%d", solver->lines_list[i + solver->space->height]->actual_colored_cells_count);
	printf("\n ");
	for (int i = 0; i < solver->space->width; i++)
		printf("/");
	printf("\n ");
	for (int i = 0; i < solver->space->width; i++)
		printf("%d", solver->lines_list[i + solver->space->height]->required_colored_cells_count);
	printf("\n ");
	for (int i = 0; i < solver->space->width; i++)
		printf("-");
	printf("\n");

	for (int y = 0; y < solver->space->height; y++) {
		printf("|");
		for (int x = 0; x < solver->space->width; x++) {
			if (solver->space->cells[x][y][1]->type == empty)
				printf(" ");
			else
				printf("X");
		}
		printf("|\n");
	}
	printf(" ");
	for (int i = 0; i < solver->space->width; i++)
		printf("-");
	printf("\n\n");
}

void print_plane_evaluations(nono_solver_t* solver) {
	printf("Vertical blocks plane evaluation: %f\n", solver->planes_list[0]->evaluation);
	printf("Horizontal blocks plane evaluation: %f\n", solver->planes_list[1]->evaluation);
}

void print_solved_nonogram(nono_solver_t* solver) {
	int width = solver->space->width;
	int height = solver->space->height;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			ca_cell_t* cell = solver->space->cells[x][y][0];
			if (cell->type == colored)
				printf("X");
			else
				printf(".");
		}
		printf("\n");
	}
}