#include "tester_v2.h"
#include "structures_v2.h"
#include "inicializator_v2.h"
#include "solver_v2.h"
#include "printer_v2.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int counter = 0;
int success_counter = 0;

void process_test(char* nono_file, int max_iterations, bool print_solution) {
    solver_v2_t* solver = init_nono(nono_file);

    if (solver == 0) {
        printf("\terror\n");
        return;
    }

    if (solve(solver, max_iterations, false) == 0) {
        printf("\tpassed\n");
        if (print_solution)
            priint_solved_nonogram(solver);
        success_counter++;
    }
    else
        printf("\tfailed\n");

    free_solver_v2(solver);
    counter++;
}

void test_v2(char* file_name, int max_iterations, bool print_solution) {
    printf("%s:", file_name);
    process_test(file_name, max_iterations, print_solution);
}

void test_set_v2(char* directory_name, int max_iterations, bool print_solution) {
    counter = 0;
    success_counter = 0;

    char list_path[512];
    snprintf(list_path, sizeof(list_path), "%s/list.txt", directory_name);

    FILE* f = fopen(list_path, "r");
    if (!f) {
        printf("Unable to open list file %s\n", list_path);
        return;
    }

    char line[256];

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0') {
            continue;
        }

        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", directory_name, line);

        test_v2(file_path, max_iterations, print_solution);
    }

    fclose(f);

    printf("\nPassed %d/%d\n\n", success_counter, counter);
}