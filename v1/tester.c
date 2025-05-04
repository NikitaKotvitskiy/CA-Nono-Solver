#include "tester.h"
#include "structures.h"
#include "inicializer.h"
#include "solver.h"
#include "printer.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int counter = 0;
int success_counter = 0;

void process_test(char* nono_file, bool print_solution) {
    nono_solver_t* solver = initialize_solver(nono_file);

    if (solver == 0) {
        printf("\terror\n");
        return;
    }

    if (solve_nono(solver, false) == true) {
        printf("\tpassed\n");
        if (print_solution)
            print_solved_nonogram(solver);
        success_counter++;
    }
    else
        printf("\tfailed\n");

    free_solver(solver);
    counter++;
}

void test(char* file_name, bool print_solution) {
    printf("%s:", file_name);
    process_test(file_name, print_solution);
}

void test_set(char* directory_name, bool print_solution) {
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

        test(file_path, print_solution);
    }

    fclose(f);

    printf("\nPassed %d/%d\n\n", success_counter, counter);
}