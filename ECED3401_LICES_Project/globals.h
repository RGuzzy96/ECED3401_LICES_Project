#pragma once

#include "map.h"
#include <time.h>

enum RUNMODE { NONE, DESIGN, EMULATOR };

typedef struct {
	int x, y, layer;
} NavStep;

#define MAX_STEPS 2048 // robot will die before taking this many steps

// bool between drawing and movement mode
extern int is_drawing_mode;

// global cave map pointer
extern Map* cave_map;

// globals for managing the offset of the console to handle map shifting
extern int viewport_x;
extern int viewport_y;

// global unsaved changes state
extern int unsaved_changes;

// run mode for toggling design and emulator
extern enum RUNMODE run_mode;

// bool state to say if we have decided to quit program from design or emulator mode
extern int quit_program;

// list of visited cells (stored as timestamps of visitation time)
extern clock_t visited_list[MAX_LAYERS][MAP_SIZE][MAP_SIZE];

// same idea as visited cells list, but for portals so we can separate
extern clock_t portal_usage_list[MAX_LAYERS][MAP_SIZE][MAP_SIZE];

// to keep only 2 layers in memory at a time
extern int active_layers[MAX_ACTIVE_LAYERS];

// optimized navigation stack for fast rescue and/or return
extern NavStep nav_stack[MAX_STEPS];
extern int nav_stack_top;