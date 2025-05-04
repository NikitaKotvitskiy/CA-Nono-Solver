#pragma once

#include "structures_v2.h"
#include <stdbool.h>

// Returns 0 if block can be placed in specified position
// Returns -1 if block cannot be placed in specified position because of min/max range exceedance
// Returns pointer to block, if cannot be placed in specified position becuase of other block (pointed)
block_v2_t* can_block_be_placed_at(solver_v2_t* solver, block_v2_t* block, int pos);

void move_block_at(solver_v2_t* solver, block_v2_t* block, int pos);