#pragma once

typedef struct pressure_alert pressure_alert_t;
typedef struct shift_alert shift_alert_t;
typedef struct empty_cell empty_cell_t;
typedef struct colored_cell colored_cell_t;
typedef struct ca_cell ca_cell_t;
typedef struct block block_t;
typedef struct line line_t;
typedef struct plane plane_t;
typedef struct ca_space ca_space_t;
typedef struct nono_solver nono_solver_t;

enum cell_type {
	empty,
	colored,
};

typedef struct pressure_alert {
	char direction;
} pressure_alert_t;

typedef struct shift_alert {
	char direction;
	double evaluation;
	block_t* block;
} shift_alert_t;

typedef struct empty_cell {
	pressure_alert_t* pressure;
	shift_alert_t* shift;
} empty_cell_t;

typedef struct colored_cell {
	char is_border_cell;
	char border_direction;
	int evaluation;
	int blockIndex;
	block_t* block;
} colored_cell_t;

typedef struct ca_cell {
	int x;
	int y;
	int z;
	enum cell_type type;
	union { 
		empty_cell_t empty_cell;
		colored_cell_t colored_cell;
	};
} ca_cell_t;

typedef struct block {
	double evaluation;
	int colored_cells_count;
	ca_cell_t** colored_cells;
	shift_alert_t* shift;
	char last_movement_direction;
} block_t;

typedef struct line {
	int required_colored_cells_count;
	int actual_colored_cells_count;
} line_t;

typedef struct plane {
	block_t** blocks_list;
	int blocks_count;
	double evaluation;
} plane_t;

typedef struct ca_space {
	int width;
	int height;
	ca_cell_t**** cells;
} ca_space_t;

typedef struct nono_solver {
	ca_space_t* space;
	plane_t** planes_list;
	block_t** blocks_list;
	line_t** lines_list;
} nono_solver_t;