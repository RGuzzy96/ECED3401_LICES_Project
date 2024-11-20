/*
 - Screen routines:
	- screen_init() - initialize screen
	- draw_object() - display symbol at col/row
	- printf_msg() - display diagnostic on last row

 - ECED 3401
 -  3 Oct 24 - Original
*/

#include "vt100.h"
#include "screen.h"
#include "map.h"
#include "globals.h"
#include "time.h" // using for debouncing resize, found at https://stackoverflow.com/questions/17167949/how-to-use-timer-in-c

#define RESIZE_DEBOUNCE_MS 1000 // debounce with 200 ms

SCREEN screen;
clock_t last_resize_time = 0;

void print_msg(char* msg, int bottom)
{
/*
 - Display message in diagnostic area (LL of active screen)
*/
/* Move to last row */
	
if (bottom == 1) {
	CUP(screen.min.col, screen.max.row)
}
else {
	CUP(screen.max.col, screen.min.row + 1)
}

/* Change BG colour */
printf(CSI "%dm", BGBLACK);
/* Erase */
if (bottom) {
	EL(screen.max.row)
}
else {
	EL(screen.min.row);
}

printf(CSI "%d;%dm%s", BGBLUE, FGWHITE, msg);
printf(CSI "%d;%dm", BGGREEN, FGWHITE); /* Restore FG & BG colour */
}

void screen_resize(int new_max_cols, int new_max_rows) {

	int old_center_x = screen.center.col;
	int old_center_y = screen.center.row;

	screen.max.col = new_max_cols;
	screen.max.row = new_max_rows;

	screen.center.col = (screen.max.col - screen.min.col) / 2;
	screen.center.row = (screen.max.row - screen.min.row) / 2;

	// get new viewport translations based on ratios of center points
	viewport_x = viewport_x + old_center_x - screen.center.col;
	viewport_y = viewport_y + old_center_y - screen.center.row;

	CLRSCR

	/* Resizing displays cursor - hide it */
	printf(CSI "?25l");

	draw_visible_map(cave_map);
}

int check_screen_size() {
	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;

	int new_max_col = 0;
	int new_max_row = 0;

	/* Get new screen size */
	GetConsoleScreenBufferInfo(scrout, &ScreenBufferInfo);

	new_max_col = ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1;
	new_max_row = ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1;

	if (new_max_col != screen.max.col || new_max_row != screen.max.row) {
		clock_t current_time = clock();
		if ((current_time - last_resize_time) * 1000 / CLOCKS_PER_SEC > RESIZE_DEBOUNCE_MS) {
			last_resize_time = current_time;
			print_msg("Adjusting screen size...", 0);
			screen_resize(new_max_col, new_max_row);
			return 1;
		}
	}
	else {
		return 0;
	}
}

void screen_init()
{
CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;

/* Get new screen size */
GetConsoleScreenBufferInfo(scrout, &ScreenBufferInfo);

screen.min.col = 1;
screen.min.row = 1;

screen.max.col = ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1;
screen.max.row = ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1;

screen.center.col = (screen.max.col - screen.min.col) / 2;
screen.center.row = (screen.max.row - screen.min.row) / 2;

#ifdef DIAG
printf("Screen details: Max: r: %d c: %d Cent: r: %d c: %d\n",
	screen.max.row, screen.max.col, screen.center.row, screen.center.col);
(void) getchar();
#endif

/* Clear the screen */
CLRSCR

/* Resizing displays cursor - hide it */
printf(CSI "?25l");

print_msg("Welcome! Please refer to run_logs.txt in the program directory for instructions.", 0);
}

int draw_object(int col, int row, int symbol)
{
/* 
 - Attempts to draw symbol at position col, row
 - Returns -1 if illegal location, 0 otherwise
*/
if (col <= screen.min.col || row <= screen.min.row ||
	col >= screen.max.col || row >= screen.max.row)
	/* Outside view screen - do not draw */
	return -1;

CUP(col, row);
_putch(symbol);

return 0;

}
