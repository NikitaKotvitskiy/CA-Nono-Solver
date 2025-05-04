#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structures_v2.h"
#include "inicializator_v2.h"
#include "printer_v2.h"
#include "solver_v2.h"
#include "tester_v2.h"
#include "cJSON.h"

bool save_solution_to_json_v2(char* path, solver_v2_t* solver, bool solved)
{
    FILE* file = fopen(path, "w");
    if (!file) {
        perror("Unable to open file");
        return false;
    }

    int width = solver->space.width;
    int height = solver->space.height;

    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "solved", solved);
    cJSON_AddNumberToObject(root, "width", width);
    cJSON_AddNumberToObject(root, "height", height);

    cJSON* solution_array = cJSON_CreateArray();

    if (solved) {
        for (int y = 0; y < height; y++) {
            cJSON* row = cJSON_CreateArray();
            for (int x = 0; x < width; x++) {
                cell_v2_t* cell = solver->space.cells[x][y][0]; // z = 0
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
        test_set_v2(path, 2000, false);
    }
    else if (strcmp(argv[1], "-solve") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Missing input or output path.\n");
            return 1;
        }

        char* in = argv[2];
        char* out = argv[3];

        solver_v2_t* solver = init_nono(in);
        if (solver == NULL) {
            fprintf(stderr, "Failed to initialize solver_v2.\n");
            return 1;
        }

        bool result = solve(solver, 2000, false);
        if (!save_solution_to_json_v2(out, solver, result == 0)) {
            fprintf(stderr, "Failed to save output.\n");
        }

        free_solver_v2(solver);
    }
    else {
        fprintf(stderr, "Unknown parameter: %s\n", argv[1]);
        return 1;
    }

    return 0;
}