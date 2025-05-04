#pragma once

#include "structures.h"

nono_solver_t* initialize_solver(char* file);
nono_solver_t* free_solver(nono_solver_t* solver);