#include "loop-detector.h"
#include "structures.h"
#include <stdlib.h>
#include <stdint.h>

static unsigned long* previous_hashes = NULL;
static size_t hash_count = 0;
static size_t capacity = 0;

static unsigned long djb2_combine(unsigned long hash, unsigned long value) {
    return ((hash << 5) + hash) ^ value;
}

static unsigned long hash_state(nono_solver_t* solver, int plane) {
    int blocks_count = solver->planes_list[0]->blocks_count + solver->planes_list[1]->blocks_count;
    unsigned long hash = 5381;

    for (int i = 0; i < blocks_count; i++) {
        block_t* block = solver->blocks_list[i];

        union { double d; unsigned long long ll; } u;
        u.d = block->evaluation;
        hash = djb2_combine(hash, (unsigned long)(u.ll & 0xFFFFFFFFUL));
        hash = djb2_combine(hash, (unsigned long)(u.ll >> 32));

        hash = djb2_combine(hash, (unsigned long)block->last_movement_direction);

        for (int j = 0; j < block->colored_cells_count; j++) {
            ca_cell_t* cell = block->colored_cells[j];
            hash = djb2_combine(hash, (unsigned long)cell->x);
            hash = djb2_combine(hash, (unsigned long)cell->y);
            hash = djb2_combine(hash, (unsigned long)cell->z);
        }
    }

    for (int p = 0; p < 2; p++) {
        union { double d; unsigned long long ll; } u_plane;
        u_plane.d = solver->planes_list[p]->evaluation;
        hash = djb2_combine(hash, (unsigned long)(u_plane.ll & 0xFFFFFFFFUL));
        hash = djb2_combine(hash, (unsigned long)(u_plane.ll >> 32));
    }

    hash = djb2_combine(hash, (unsigned long)plane);

    return hash;
}

char check_loop(nono_solver_t* solver, int plane) {
    unsigned long current_hash = hash_state(solver, plane);

    for (size_t i = 0; i < hash_count; i++) {
        if (previous_hashes[i] == current_hash)
            return 1;
    }

    if (hash_count >= capacity) {
        capacity = (capacity == 0) ? 1024 : capacity * 2;
        unsigned long* temp = realloc(previous_hashes, capacity * sizeof(unsigned long));
        if (!temp) {
            return 0;
        }
        previous_hashes = temp;
    }
    previous_hashes[hash_count++] = current_hash;

    return 0;
}

void reset_loop_detection() {
    if (previous_hashes) {
        free(previous_hashes);
    }
    previous_hashes = NULL;
    hash_count = 0;
    capacity = 0;
}
