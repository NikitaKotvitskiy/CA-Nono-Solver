#include "structures_v2.h"
#include "printer_v2.h"
#include <stdio.h>

void print_solver_state(solver_v2_t* solver) {
    int width = solver->space.width;
    int height = solver->space.height;

    printf("Vertical plane (plane 0):\n");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            cell_v2_t* cell = solver->space.cells[x][y][0];
            if (cell->type == colored && solver->space.cells[x][y][1]->type == empty)
                printf("X");
            else if (cell->type == colored && solver->space.cells[x][y][1]->type == colored)
                printf("O");
            else
                printf(".");
        }
        printf("\n");
    }
    printf("\n");

    printf("Horizontal plane (plane 1):\n");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            cell_v2_t* cell = solver->space.cells[x][y][1];
            if (cell->type == colored && solver->space.cells[x][y][0]->type == empty)
                printf("X");
            else if (cell->type == colored && solver->space.cells[x][y][0]->type == colored)
                printf("O");
            else
                printf(".");
        }
        printf("\n");
    }
    printf("\n");
    printf("Current evaluation: %f\n", solver->analyzer.current_evaluation);
    printf("Local maximum: %f\n", solver->analyzer.local_maximum);
    printf("Counter: %d\n", solver->analyzer.counter);
}

void priint_solved_nonogram(solver_v2_t* solver) {
    int width = solver->space.width;
    int height = solver->space.height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            cell_v2_t* cell = solver->space.cells[x][y][0];
            if (cell->type == colored)
                printf("X");
            else
                printf(".");
        }
        printf("\n");
    }
}