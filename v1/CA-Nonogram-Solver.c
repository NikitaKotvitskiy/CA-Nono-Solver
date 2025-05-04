#include "structures.h"
#include "inicializer.h"
#include "printer.h"
#include "solver.h"
#include "printer.h"
#include "tester.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "cJSON.h"
#include <string.h>

bool save_solution_to_json(char* path, nono_solver_t* solver, bool solved)
{
    FILE* file = fopen(path, "w");
    if (!file) {
        perror("Unable to open file for writing");
        return false;
    }

    int width = solver->space->width;
    int height = solver->space->height;

    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "solved", solved);
    cJSON_AddNumberToObject(root, "width", width);
    cJSON_AddNumberToObject(root, "height", height);

    cJSON* solution_array = cJSON_CreateArray();

    if (solved) {
        for (int y = 0; y < height; y++) {
            cJSON* row = cJSON_CreateArray();
            for (int x = 0; x < width; x++) {
                ca_cell_t* cell = solver->space->cells[x][y][0];
                int value = (cell->type == colored) ? 1 : 0;
                cJSON_AddItemToArray(row, cJSON_CreateNumber(value));
            }
            cJSON_AddItemToArray(solution_array, row);
        }
    }

    cJSON_AddItemToObject(root, "solution", solution_array);

    char* json_string = cJSON_Print(root);
    fprintf(file, "%s\n", json_string);

    fclose(file);
    free(json_string);
    cJSON_Delete(root);

    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Program usage:\n");
        fprintf(stderr, "\t-test <path_to_test_set>\n");
        fprintf(stderr, "\t-solve <path_to_nono_config> <output_path>\n");
        return 1;
    }

    if (strcmp(argv[1], "-test") == 0) {
        char* path = argv[2];
        test_set(path, false);
    }
    else if (strcmp(argv[1], "-solve") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Path to nonogram config is missing.\n");
            return 1;
        }

        char* in = argv[2];
        char* out = argv[3];

        nono_solver_t* solver = initialize_solver(in);
        if (solver == 0)
            return 1;
        if (solve_nono(solver, false) == true) {
            if (!save_solution_to_json(out, solver, true))
                fprintf(stderr, "Failed to save solution.\n");
        }
        else {
            if (!save_solution_to_json(out, solver, false))
                fprintf(stderr, "Failed to save solution.\n");
        }
        free_solver(solver);
    }
    else {
        fprintf(stderr, "Unknow parameter: %s\n", argv[1]);
        return 1;
    }

    return 0;
}