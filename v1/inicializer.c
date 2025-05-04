#include "structures.h"
#include "inicializer.h"

#include "cJSON.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include "structures.h"
#include "inicializer.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

nono_solver_t* initialize_solver(char* file) {
    FILE* fp = fopen(file, "r");
    if (!fp) {
        fprintf(stderr, "Error: Unable to open file %s\n", file);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);
    char* json_data = (char*)malloc(filesize + 1);
    if (!json_data) {
        fclose(fp);
        return NULL;
    }
    size_t ret = fread(json_data, 1, filesize, fp);
    json_data[filesize] = '\0';
    fclose(fp);

    cJSON* root = cJSON_Parse(json_data);
    free(json_data);
    if (!root) {
        fprintf(stderr, "Error: Unable to parse JSON\n");
        return NULL;
    }

    cJSON* width_json = cJSON_GetObjectItem(root, "width");
    cJSON* height_json = cJSON_GetObjectItem(root, "height");
    if (!width_json || !height_json) {
        cJSON_Delete(root);
        return NULL;
    }
    int width = width_json->valueint;
    int height = height_json->valueint;

    nono_solver_t* solver = (nono_solver_t*)malloc(sizeof(nono_solver_t));
    if (!solver) {
        cJSON_Delete(root);
        return NULL;
    }
    solver->space = NULL;
    solver->planes_list = NULL;
    solver->blocks_list = NULL;
    solver->lines_list = NULL;

    solver->space = (ca_space_t*)malloc(sizeof(ca_space_t));
    solver->space->width = width;
    solver->space->height = height;
    solver->space->cells = (ca_cell_t****)malloc(width * sizeof(ca_cell_t***));
    for (int x = 0; x < width; x++) {
        solver->space->cells[x] = (ca_cell_t***)malloc(height * sizeof(ca_cell_t**));
        for (int y = 0; y < height; y++) {
            solver->space->cells[x][y] = (ca_cell_t**)malloc(2 * sizeof(ca_cell_t*));
            for (int z = 0; z < 2; z++) {
                solver->space->cells[x][y][z] = (ca_cell_t*)malloc(sizeof(ca_cell_t));
                solver->space->cells[x][y][z]->x = x;
                solver->space->cells[x][y][z]->y = y;
                solver->space->cells[x][y][z]->z = z;
                solver->space->cells[x][y][z]->type = empty;
                solver->space->cells[x][y][z]->empty_cell.shift = NULL;
                solver->space->cells[x][y][z]->empty_cell.pressure = NULL;
            }
        }
    }

    cJSON* vertical_array = cJSON_GetObjectItem(root, "vertical_blocks_lengths");
    cJSON* horizontal_array = cJSON_GetObjectItem(root, "horizontal_blocks_lengths");
    if (!vertical_array || !horizontal_array) {
        cJSON_Delete(root);
        return NULL;
    }

    int vertical_lines = cJSON_GetArraySize(vertical_array);
    int horizontal_lines = cJSON_GetArraySize(horizontal_array);

    int total_vertical_blocks = 0;
    for (int i = 0; i < vertical_lines; i++) {
        cJSON* col_blocks = cJSON_GetArrayItem(vertical_array, i);
        total_vertical_blocks += cJSON_GetArraySize(col_blocks);
    }

    int total_horizontal_blocks = 0;
    for (int i = 0; i < horizontal_lines; i++) {
        cJSON* row_blocks = cJSON_GetArrayItem(horizontal_array, i);
        total_horizontal_blocks += cJSON_GetArraySize(row_blocks);
    }

    int total_lines = vertical_lines + horizontal_lines;
    int total_blocks = total_vertical_blocks + total_horizontal_blocks;

    solver->blocks_list = (block_t**)malloc(total_blocks * sizeof(block_t*));
    solver->lines_list = (line_t**)malloc(total_lines * sizeof(line_t*));

    solver->planes_list = (plane_t**)malloc(2 * sizeof(plane_t*));
    solver->planes_list[0] = (plane_t*)malloc(sizeof(plane_t));
    solver->planes_list[1] = (plane_t*)malloc(sizeof(plane_t));

    solver->planes_list[0]->evaluation = 0;
    solver->planes_list[0]->blocks_count = total_vertical_blocks;
    solver->planes_list[1]->evaluation = 0;
    solver->planes_list[1]->blocks_count = total_horizontal_blocks;

    solver->planes_list[0]->blocks_list = solver->blocks_list;
    solver->planes_list[1]->blocks_list = solver->blocks_list + total_vertical_blocks;

    int global_block_index = 0;
    int vb_index = 0;
    int global_line_index = 0;
    int hb_index = 0;

    for (int col = 0; col < vertical_lines; col++) {
        cJSON* line_blocks = cJSON_GetArrayItem(horizontal_array, col);
        if (!line_blocks) {
            line_t* empty_line = (line_t*)malloc(sizeof(line_t));
            empty_line->required_colored_cells_count = 0;
            empty_line->actual_colored_cells_count = 0;
            solver->lines_list[global_line_index++] = empty_line;
        }
        else {
            int num_blocks = cJSON_GetArraySize(line_blocks);

            line_t* line = (line_t*)malloc(sizeof(line_t));
            int required = 0;
            for (int j = 0; j < num_blocks; j++) {
                cJSON* block_val = cJSON_GetArrayItem(line_blocks, j);
                required += block_val->valueint;
            }
            line->required_colored_cells_count = required;
            line->actual_colored_cells_count = 0;
            solver->lines_list[global_line_index++] = line;
        }

        cJSON* col_blocks = cJSON_GetArrayItem(vertical_array, col);
        int vblock_count = (col_blocks ? cJSON_GetArraySize(col_blocks) : 0);

        int pos = 0;
        for (int j = 0; j < vblock_count; j++) {
            cJSON* block_val = cJSON_GetArrayItem(col_blocks, j);
            int block_length = block_val->valueint;

            block_t* block = (block_t*)malloc(sizeof(block_t));
            block->evaluation = 0;
            block->colored_cells_count = block_length;
            block->colored_cells = (ca_cell_t**)malloc(block_length * sizeof(ca_cell_t*));
            block->shift = NULL;
            block->last_movement_direction = 0;

            for (int k = 0; k < block_length; k++) {
                if (pos + k >= height) break;
                ca_cell_t* cell = solver->space->cells[col][pos + k][0];
                cell->type = colored;
                cell->colored_cell.is_border_cell = (k == 0 || k == block_length - 1) ? 1 : 0;
                cell->colored_cell.evaluation = 0;
                cell->colored_cell.blockIndex = k;
                cell->colored_cell.block = block;

                if (cell->colored_cell.is_border_cell) {
                    if (block_length == 1)
                        cell->colored_cell.border_direction = 0;
                    else if (k == 0)
                        cell->colored_cell.border_direction = -1;
                    else if (k == block_length - 1)
                        cell->colored_cell.border_direction = 1;
                }
                block->colored_cells[k] = cell;
            }
            pos += block_length + 1;

            solver->blocks_list[global_block_index] = block;
            solver->planes_list[0]->blocks_list[vb_index] = block;
            global_block_index++;
            vb_index++;
        }
    }

    for (int row = 0; row < horizontal_lines; row++) {
        cJSON* line_blocks = cJSON_GetArrayItem(vertical_array, row);
        if (!line_blocks) {
            line_t* empty_line = (line_t*)malloc(sizeof(line_t));
            empty_line->required_colored_cells_count = 0;
            empty_line->actual_colored_cells_count = 0;
            solver->lines_list[global_line_index++] = empty_line;
        }
        else {
            int num_blocks = cJSON_GetArraySize(line_blocks);
            line_t* line = (line_t*)malloc(sizeof(line_t));
            int required = 0;
            for (int j = 0; j < num_blocks; j++) {
                cJSON* block_val = cJSON_GetArrayItem(line_blocks, j);
                required += block_val->valueint;
            }
            line->required_colored_cells_count = required;
            line->actual_colored_cells_count = 0;
            solver->lines_list[global_line_index++] = line;
        }

        cJSON* row_blocks = cJSON_GetArrayItem(horizontal_array, row);
        int hblock_count = (row_blocks ? cJSON_GetArraySize(row_blocks) : 0);

        int pos = 0;
        for (int j = 0; j < hblock_count; j++) {
            cJSON* block_val = cJSON_GetArrayItem(row_blocks, j);
            int block_length = block_val->valueint;

            block_t* block = (block_t*)malloc(sizeof(block_t));
            block->evaluation = 0;
            block->colored_cells_count = block_length;
            block->colored_cells = (ca_cell_t**)malloc(block_length * sizeof(ca_cell_t*));
            block->shift = NULL;
            block->last_movement_direction = 0;

            for (int k = 0; k < block_length; k++) {
                if (pos + k >= width) break;
                ca_cell_t* cell = solver->space->cells[pos + k][row][1];
                cell->type = colored;
                cell->colored_cell.is_border_cell = (k == 0 || k == block_length - 1) ? 1 : 0;
                cell->colored_cell.border_direction = 0;
                cell->colored_cell.evaluation = 0;
                cell->colored_cell.blockIndex = k;
                cell->colored_cell.block = block;
                block->colored_cells[k] = cell;
            }
            pos += block_length + 1;

            solver->blocks_list[global_block_index] = block;
            solver->planes_list[1]->blocks_list[hb_index] = block;
            global_block_index++;
            hb_index++;
        }
    }

    cJSON_Delete(root);
    return solver;
}

nono_solver_t* free_solver(nono_solver_t* solver) {
    if (!solver)
        return NULL;

    int width = solver->space ? solver->space->width : 0;
    int height = solver->space ? solver->space->height : 0;

    if (solver->lines_list) {
        int total_lines = width + height;
        for (int i = 0; i < total_lines; i++) {
            free(solver->lines_list[i]);
            solver->lines_list[i] = NULL;
        }
        free(solver->lines_list);
        solver->lines_list = NULL;
    }

    if (solver->space) {
        if (solver->space->cells) {
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    for (int z = 0; z < 2; z++) {
                        free(solver->space->cells[x][y][z]);
                        solver->space->cells[x][y][z] = NULL;
                    }
                    free(solver->space->cells[x][y]);
                    solver->space->cells[x][y] = NULL;
                }
                free(solver->space->cells[x]);
                solver->space->cells[x] = NULL;
            }
            free(solver->space->cells);
            solver->space->cells = NULL;
        }
        free(solver->space);
        solver->space = NULL;
    }

    if (solver->blocks_list) {
        int total_blocks = 0;
        if (solver->planes_list) {
            if (solver->planes_list[0])
                total_blocks += solver->planes_list[0]->blocks_count;
            if (solver->planes_list[1])
                total_blocks += solver->planes_list[1]->blocks_count;
        }
        for (int i = 0; i < total_blocks; i++) {
            if (solver->blocks_list[i]) {
                free(solver->blocks_list[i]->colored_cells);
                solver->blocks_list[i]->colored_cells = NULL;
                free(solver->blocks_list[i]);
                solver->blocks_list[i] = NULL;
            }
        }
        free(solver->blocks_list);
        solver->blocks_list = NULL;
    }

    if (solver->planes_list) {
        if (solver->planes_list[0]) {
            free(solver->planes_list[0]);
            solver->planes_list[0] = NULL;
        }
        if (solver->planes_list[1]) {
            free(solver->planes_list[1]);
            solver->planes_list[1] = NULL;
        }
        free(solver->planes_list);
        solver->planes_list = NULL;
    }

    free(solver);
    return NULL;
}
