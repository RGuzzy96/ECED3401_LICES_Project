#pragma once
/* 
 - Basic robot structure and entry points
 - ECED 3401
	- 3 Oct 24 - Original (Dr. Larry Hughes)
	- 1 Nov 24 - Modified for Task 5 implementation (Ryan Guzzwell, Hemang Arora)
*/

// need to store robot state for battery and radiation exposure or whatever

struct robot
{
int x;
int y;
int curr_dir;	/* Current direction N, S, E, W */
/* Restore things if overshoot */
int oldx;
int oldy;
int old_dir;
};

typedef struct robot ROBOT;

extern void robot_init();
extern void robot_move(char, int offset_x, int offset_y);

extern void create_portal(char portal_direction);
extern void use_portal();
extern void set_cell_attributes();

/* In system.c: */
extern void go_robot_go();
