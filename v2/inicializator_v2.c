#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cJSON.h"
#include "float.h"

#include "inicializator_v2.h"
#include "structures_v2.h"

typedef struct block_info {
    int length;
    block_v2_t* pointer;
} block_info_t;

int init_lines(solver_v2_t* solver) {
    int plane = 0;

    for (int x = 0; x < solver->space.width; x++) {
        int max_blocks_count = (int)((double)solver->space.height / 2 + 0.5);
        block_info_t* blocks_info = malloc(sizeof(block_info_t) * max_blocks_count);
        if (max_blocks_count == 0) {
            fprintf(stderr, "Memory allocation error\n");
            return -1;
        }
        int block_index = 0;

        for (int y = 0; y < solver->space.height; y++) {
            cell_v2_t* cell = solver->space.cells[x][y][plane];
            if (cell->type == colored) {
                blocks_info[block_index].length = cell->colored.block->cells_count;
                blocks_info[block_index].pointer = cell->colored.block;
                y += cell->colored.block->cells_count;
                block_index++;
                continue;
            }
            break;
        }

        for (int i = 0; i < block_index; i++) {
            int min = 0;
            int max = 0;
            for (int a = 0; a < i; a++)
                min += blocks_info[a].length + 1;
            for (int b = i + 1; b < block_index; b++)
                max += 1 + blocks_info[b].length;
            blocks_info[i].pointer->min_possible_coord = min;
            blocks_info[i].pointer->max_possible_coord = solver->space.height - max - blocks_info[i].length;
        }

        free(blocks_info);
    }

    plane = 1;
    
    for (int y = 0; y < solver->space.height; y++) {
        int max_blocks_count = (int)((double)solver->space.width / 2 + 0.5);
            block_info_t* blocks_info = malloc(sizeof(block_info_t) * max_blocks_count);
        if (max_blocks_count == 0) {
            fprintf(stderr, "Memory allocation error\n");
            return -1;
        }
        int block_index = 0;

        for (int x = solver->space.width - 1; x >= 0; x--) {
            cell_v2_t* cell = solver->space.cells[x][y][plane];
            if (cell->type == colored) {
                blocks_info[block_index].length = cell->colored.block->cells_count;
                blocks_info[block_index].pointer = cell->colored.block;
                x -= cell->colored.block->cells_count;
                block_index++;
                continue;
            }
            break;
        }

        for (int i = 0; i < block_index; i++) {
            int min = 0;
            int max = 0;
            for (int a = 0; a < i; a++)
                max += blocks_info[a].length + 1;
            for (int b = i + 1; b < block_index; b++)
                min += 1 + blocks_info[b].length;
            blocks_info[i].pointer->min_possible_coord = min;
            blocks_info[i].pointer->max_possible_coord = solver->space.width - max - blocks_info[i].length;
        }

        free(blocks_info);
    }

    for (int i = 0; i < solver->space.vertical_blocks_count + solver->space.horizontal_blocks_count; i++) {
        block_v2_t* block = solver->space.blocks[i];
        int plane = block->cells[0]->coords.z;
        for (int j = 0; j < block->cells_count; j++) {
            cell_v2_t* cell = block->cells[j];
            cell->colored.min = block->min_possible_coord;
            cell->colored.max = block->max_possible_coord + block->cells_count - 1;
        }
    }

    return 0;
}

solver_v2_t* init_nono(char* file_name) {
    FILE* f = fopen(file_name, "rb");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);

    char* json_data = malloc(file_size + 1);
    if (!json_data) {
        fclose(f);
        return NULL;
    }
    size_t read_size = fread(json_data, 1, file_size, f);
    fclose(f);
    json_data[read_size] = '\0';

    cJSON* root = cJSON_Parse(json_data);
    free(json_data);
    if (!root) {
        fprintf(stderr, "Error parsing JSON\n");
        return NULL;
    }

    solver_v2_t* solver = malloc(sizeof(solver_v2_t));
    if (!solver) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON* json_width = cJSON_GetObjectItem(root, "width");
    cJSON* json_height = cJSON_GetObjectItem(root, "height");
    solver->space.width = json_width ? json_width->valueint : 0;
    solver->space.height = json_height ? json_height->valueint : 0;
    solver->space.vertical_blocks_count = 0;
    solver->space.horizontal_blocks_count = 0;

    int planes = 2;
    solver->space.cells = malloc(solver->space.width * sizeof(cell_v2_t***));
    if (!solver->space.cells) {
        free(solver);
        cJSON_Delete(root);
        return NULL;
    }
    for (int x = 0; x < solver->space.width; x++) {
        solver->space.cells[x] = malloc(solver->space.height * sizeof(cell_v2_t**));
        if (!solver->space.cells[x]) {
            for (int k = 0; k < x; k++) {
                free(solver->space.cells[k]);
            }
            free(solver->space.cells);
            free(solver);
            cJSON_Delete(root);
            return NULL;
        }
        for (int y = 0; y < solver->space.height; y++) {
            solver->space.cells[x][y] = malloc(planes * sizeof(cell_v2_t*));
            if (!solver->space.cells[x][y]) {
                for (int j = 0; j < y; j++) {
                    free(solver->space.cells[x][j]);
                }
                free(solver->space.cells[x]);
                for (int k = 0; k < x; k++) {
                    free(solver->space.cells[k]);
                }
                free(solver->space.cells);
                free(solver);
                cJSON_Delete(root);
                return NULL;
            }
            for (int z = 0; z < planes; z++) {
                solver->space.cells[x][y][z] = malloc(sizeof(cell_v2_t));
                if (!solver->space.cells[x][y][z]) {
                    continue;
                }
                solver->space.cells[x][y][z]->coords.x = x;
                solver->space.cells[x][y][z]->coords.y = y;
                solver->space.cells[x][y][z]->coords.z = z;
                solver->space.cells[x][y][z]->type = empty;
                solver->space.cells[x][y][z]->colored.force_to_move = 0.0;
                solver->space.cells[x][y][z]->colored.force_to_stay = 0.0;
                solver->space.cells[x][y][z]->colored.block = NULL;
            }
        }
    }

    cJSON* vertical_blocks_array = cJSON_GetObjectItem(root, "vertical_blocks_lengths");
    cJSON* horizontal_blocks_array = cJSON_GetObjectItem(root, "horizontal_blocks_lengths");

    int vertical_columns = cJSON_GetArraySize(vertical_blocks_array);
    int vertical_total = 0;
    for (int i = 0; i < vertical_columns; i++) {
        cJSON* col_array = cJSON_GetArrayItem(vertical_blocks_array, i);
        vertical_total += cJSON_GetArraySize(col_array);
    }

    int horizontal_rows = cJSON_GetArraySize(horizontal_blocks_array);
    int horizontal_total = 0;
    for (int i = 0; i < horizontal_rows; i++) {
        cJSON* row_array = cJSON_GetArrayItem(horizontal_blocks_array, i);
        horizontal_total += cJSON_GetArraySize(row_array);
    }

    int total_blocks = vertical_total + horizontal_total;
    solver->space.blocks = malloc(total_blocks * sizeof(block_v2_t*));
    if (!solver->space.blocks) {
        free(solver);
        cJSON_Delete(root);
        return NULL;
    }

    solver->space.vertical_blocks_count = vertical_total;
    solver->space.horizontal_blocks_count = horizontal_total;

    int block_index = 0;

    for (int col = 0; col < vertical_columns; col++) {
        cJSON* col_array = cJSON_GetArrayItem(vertical_blocks_array, col);
        int num_blocks = cJSON_GetArraySize(col_array);
        int current_row = 0;
        for (int b = 0; b < num_blocks; b++) {
            cJSON* length_item = cJSON_GetArrayItem(col_array, b);
            int length = length_item->valueint;
            if (current_row + length > solver->space.height) {
                fprintf(stderr, "Vertical block in column %d does not fit in the grid.\n", col);
                continue;
            }
            block_v2_t* block = malloc(sizeof(block_v2_t));
            if (!block) {
                continue;
            }
            block->cells_count = length;
            block->force_to_move = 0.0;
            block->force_to_stay = 0.0;
            block->gravity_power = 1.0;
            block->cells = malloc(length * sizeof(cell_v2_t*));
            if (!block->cells) {
                free(block);
                continue;
            }
            for (int k = 0; k < length; k++) {
                block->cells[k] = solver->space.cells[col][current_row + k][0];
                solver->space.cells[col][current_row + k][0]->colored.block = block;
                solver->space.cells[col][current_row + k][0]->colored.gravity_power = 1.0;
                solver->space.cells[col][current_row + k][0]->type = colored;
            }
            solver->space.blocks[block_index++] = block;
            current_row += length + 1;
        }
    }

    for (int row = 0; row < horizontal_rows; row++) {
        int length = cJSON_GetArraySize(horizontal_blocks_array);
        cJSON* row_array = cJSON_GetArrayItem(horizontal_blocks_array, row);

        int num_blocks = cJSON_GetArraySize(row_array);
        int current_col = solver->space.width - 1;
        for (int b = num_blocks - 1; b >= 0; b--) {
            cJSON* length_item = cJSON_GetArrayItem(row_array, b);
            int length = length_item->valueint;
            if (current_col - length + 1 < 0) {
                fprintf(stderr, "Horizontal block in row %d does not fit in the grid.\n", row);
                continue;
            }
            block_v2_t* block = malloc(sizeof(block_v2_t));
            if (!block) {
                continue;
            }
            block->cells_count = length;
            block->force_to_move = 0.0;
            block->force_to_stay = 0.0;
            block->gravity_power = 1.0;
            block->cells = malloc(length * sizeof(cell_v2_t*));
            if (!block->cells) {
                free(block);
                continue;
            }

            for (int k = 0; k < length; k++) {
                block->cells[k] = solver->space.cells[current_col - k][row][1];
                solver->space.cells[current_col - k][row][1]->colored.block = block;
                solver->space.cells[current_col - k][row][1]->colored.gravity_power = 1.0;
                solver->space.cells[current_col - k][row][1]->type = colored;
            }
            solver->space.blocks[block_index++] = block;
            current_col -= (length + 1);
        }
    }

    // Инициализация анализатора
    solver->analyzer.counter = 0;
    solver->analyzer.local_maximum = -DBL_MAX;
    solver->analyzer.current_evaluation = 0.0;

    cJSON_Delete(root);
    if (init_lines(solver) == -1) {
        free_solver_v2(solver);
        return 0;
    }
    return solver;
}

void free_solver_v2(solver_v2_t* solver) {
    if (!solver) {
        return;
    }

    for (int i = 0; i < solver->space.horizontal_blocks_count + solver->space.vertical_blocks_count; i++) {
        free(solver->space.blocks[i]->cells);
        free(solver->space.blocks[i]);
    }
    free(solver->space.blocks);

    for (int x = 0; x < solver->space.width; x++) {
        for (int y = 0; y < solver->space.height; y++) {
            for (int z = 0; z < 2; z++)
                free(solver->space.cells[x][y][z]);
            free(solver->space.cells[x][y]);
        }
        free(solver->space.cells[x]);
    }
    free(solver->space.cells);

    free(solver);
}