#pragma once
/*
 - Define basic screen structures
 - Need row min and max, and col min and max
 - Center needed for starting robot
 - ECED 3401
	- 3 Oct 24 - Original (Dr. Larry Hughes)
	- 1 Nov 24 - Modified for Task 5 implementation (Ryan Guzzwell, Hemang Arora)
*/

typedef struct scr_coord
{
int col;
int row;
}SCR_COORD;

typedef struct screen
{
SCR_COORD min;
SCR_COORD max;
SCR_COORD center;
} SCREEN;

extern SCREEN screen;

/* Entry points for functions */
extern int draw_object(int col, int row, int symbol);
extern void print_msg(char* msg, int bottom);
extern void screen_init();
extern int check_screen_size();