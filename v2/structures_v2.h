#pragma once

#include <stdbool.h>

typedef struct coords_v2 coords_v2_t;
typedef struct colored_cell_v2 colored_cell_v2_t;
typedef struct block_v2 block_v2_t;
typedef struct space_v2 space_v2_t;
typedef struct cell_v2 cell_v2_t;
typedef struct analyzer_v2 analyzer_v2_t;
typedef struct solver_v2 solver_v2_t;

enum cell_type_v2 {
	empty,
	colored,
};

typedef struct coords_v2 {
	int x;
	int y;
	int z;
} coords_v2_t;

typedef struct colored_cell_v2 {
	double force_to_move;
	double force_to_stay;
	double gravity_power;
	block_v2_t* block;
	int min;
	int max;
} colored_cell_v2_t;

typedef struct cell_v2 {
	coords_v2_t coords;
	enum cell_type_v2 type;
	colored_cell_v2_t colored;
} cell_v2_t;

typedef struct block_v2 {
	cell_v2_t** cells;
	int cells_count;
	double force_to_move;
	double force_to_stay;
	double gravity_power;
	int move_index;
	bool complete_match;
	int min_possible_coord;
	int max_possible_coord;
} block_v2_t;

typedef struct space_v2 {
	block_v2_t** blocks;
	cell_v2_t**** cells;
	int width;
	int height;
	int vertical_blocks_count;
	int horizontal_blocks_count;
} space_v2_t;

typedef struct analyzer_v2 {
	int counter;
	double local_maximum;
	double current_evaluation;
} analyzer_v2_t;

typedef struct solver_v2 {
	space_v2_t space;
	analyzer_v2_t analyzer;
} solver_v2_t;