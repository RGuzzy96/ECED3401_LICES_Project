#pragma once

#include "map.h"

enum RUNMODE { NONE, DESIGN, EMULATOR };

extern int is_drawing_mode;
extern 	Map* cave_map;
extern int viewport_x;
extern int viewport_y;
extern int unsaved_changes;
extern enum RUNMODE run_mode;
extern int quit_program;
extern int visited_list[MAX_LAYERS][MAP_SIZE][MAP_SIZE];
extern int portal_usage_list[MAX_LAYERS][MAP_SIZE][MAP_SIZE];