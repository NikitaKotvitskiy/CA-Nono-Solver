#include "structures_v2.h"
#include "solver_v2.h"
#include "printer_v2.h"
#include "helper_v2.h"
#include "evaluator_v2.h"
#include "shifter_v2.h"

#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

block_v2_t** randomize_blocks(solver_v2_t* solver) {
    int total_blocks = solver->space.vertical_blocks_count + solver->space.horizontal_blocks_count;

    block_v2_t** random_order = malloc(total_blocks * sizeof(block_v2_t*));
    if (!random_order) {
        fprintf(stderr, "Memory allocation failed in process_blocks_shifts.\n");
        return 0;
    }

    for (int i = 0; i < total_blocks; i++) {
        random_order[i] = solver->space.blocks[i];
    }

    // Fisher-Yates shuffle
    for (int i = total_blocks - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        block_v2_t* temp = random_order[i];
        random_order[i] = random_order[j];
        random_order[j] = temp;
    }

    for (int i = 0; i < total_blocks; i++) {
        random_order[i]->move_index = i;
    }

    return random_order;
}

void process_blocks_shifts(solver_v2_t* solver) {
    block_v2_t** order = randomize_blocks(solver);
    
    for (int i = 0; i < solver->space.horizontal_blocks_count + solver->space.vertical_blocks_count; i++) {
        block_v2_t* block = order[i];
        evaluate_block_v2(solver, block);
        if (fabs(block->force_to_move) < block->force_to_stay)
            continue;

        int move_direction = (block->force_to_move > 0) ? 1 : -1;
        int plane = block->cells[0]->coords.z;
        int current_block_coord;
        if (plane == 0)
            current_block_coord = block->cells[0]->coords.y;
        else
            current_block_coord = block->cells[block->cells_count - 1]->coords.x;
        
        block_v2_t* obstacle = can_block_be_placed_at(solver, block, current_block_coord + move_direction);
        if (obstacle == (block_v2_t*)-1)
            continue;
        if (obstacle == 0) {
            move_block_at(solver, block, current_block_coord + move_direction);
            continue;
        }
        
        int obstacle_move_direction;
        if (obstacle->force_to_stay > obstacle->force_to_move)
            obstacle_move_direction = 0;
        else
            obstacle_move_direction = (obstacle->force_to_move > 0) ? 1 : -1;

        if (obstacle_move_direction == move_direction) {
            order[i] = obstacle;
            order[obstacle->move_index] = block;
            int tmp = obstacle->move_index;
            obstacle->move_index = block->move_index;
            block->move_index = tmp;
            i--;
        }
    }

    free(order);
}


void move_blocks_randomly(solver_v2_t* solver) {
    block_v2_t** order = randomize_blocks(solver);

    for (int i = 0; i < solver->space.horizontal_blocks_count + solver->space.vertical_blocks_count; i++) {
        block_v2_t* block = order[i];

        if (block->max_possible_coord == block->min_possible_coord)
            continue;
        int* possible_coords = malloc((block->max_possible_coord - block->min_possible_coord + 1) * sizeof(int));
        if (possible_coords == 0) {
            fprintf(stderr, "Memory allocation error");
            continue;
        }
        int count_of_possibe_coords = 0;

        for (int j = block->min_possible_coord; j <= block->max_possible_coord; j++)
            if (can_block_be_placed_at(solver, block, j) == 0) {
                possible_coords[count_of_possibe_coords] = j;
                count_of_possibe_coords++;
            }

        if (count_of_possibe_coords == 0)
            continue;

        int randomIndex = rand() % count_of_possibe_coords;
        move_block_at(solver, block, possible_coords[randomIndex]);

        free(possible_coords);
    }

    free(order);
}

void clear_forces(solver_v2_t* solver) {
    for (int i = 0; i < solver->space.horizontal_blocks_count + solver->space.vertical_blocks_count; i++) {
        block_v2_t* block = solver->space.blocks[i];
        for (int j = 0; j < block->cells_count; j++) {
            cell_v2_t* cell = block->cells[j];
            cell->colored.gravity_power = 1.0;
        }
        block->gravity_power = 1.0;
    }
}

bool update_stats(solver_v2_t* solver) {
    bool solved = true;

    int width = solver->space.width;
    int height = solver->space.height;
    int current_evaluation = 0;

    for (int i = 0; i < solver->space.horizontal_blocks_count + solver->space.vertical_blocks_count; i++) {
        block_v2_t* block = solver->space.blocks[i];
        block->complete_match = true;
        for (int j = 0; j < block->cells_count; j++) {
            cell_v2_t* cell = block->cells[j];
            cell_v2_t* op_cell = solver->space.cells[cell->coords.x][cell->coords.y][cell->coords.z ? 0 : 1];
            if (op_cell->type == colored)
                current_evaluation += 1;
            else {
                cell->colored.gravity_power *= CELL_GRAVITY_POWER_INCREASE_COEFFICIENT;
                block->complete_match = false;
                solved = false;
            }
        }

        if (block->complete_match)
            block->gravity_power = 1.0;
        else
            block->gravity_power *= BLOCK_GRAVITY_POWER_INCREASE_COEFFICIENT;
    }

    if (current_evaluation > solver->analyzer.local_maximum) {
        solver->analyzer.local_maximum = current_evaluation;
        solver->analyzer.counter = 0;
    }
    else {
        solver->analyzer.counter++;
        if (solver->analyzer.counter > MAX_STEPS_WITHOUT_PROGRESS) {
            move_blocks_randomly(solver);
            solver->analyzer.counter = 0;
        }
    }

    solver->analyzer.current_evaluation = current_evaluation;
    return solved;
}

int solve(solver_v2_t* solver, int max_iteration_count, bool debug) {
    int counter = 0;

    if (debug) print_solver_state(solver);
    while (counter < max_iteration_count) {
        process_blocks_shifts(solver);

        bool solved = update_stats(solver);
        if (debug) print_solver_state(solver);
        if (solved)
            return 0;

        counter++;
    }
    return -1;
}