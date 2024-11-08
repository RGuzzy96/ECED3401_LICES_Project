/*

ECED3401 Systems Analysis
Lunar Ice Cave Explorer (LICE) Project
Current Stage: Task 5

Authors:
Ryan Guzzwell (B00859689) and Hemang Arora (B00909334)

Overview:

This program is part of the larger Lunar Ice Cave Explorer (LICE) project, which aims to design software
for the Canadian Space Agency's LICER rover. The LICER will autonomously explore lunar caves, map them,
and search for ice. 

This implementation at its current stage specifically addresses Task 5, which focuses on creating a
cave mapping tool. The maps generated here will later be used by the LICER to navigate and find ice 
(in Task 7) as well as stored for future retrieval (Task 6).

Date: November 1, 2024

Changelog:
- 3 Oct 24 - Original (Dr. Larry Hughes)
- 1 Nov 24 - Modified for Task 5 implementation (Ryan Guzzwell, Hemang Arora)

*/

#include <stdio.h>
#include <stdlib.h>

#include "VT100.h"
#include "screen.h"
#include "robot.h"
#include "map.h"
#include "logger.h"
#include "screen.h"

HANDLE scrout; // output stream handler (screen output)
HANDLE kbin; // input stream handler (keyboard input)

int is_drawing_mode = 0; // initialize robot in move mode
Map* cave_map = NULL;

void terminate(char* msg) {
	// fatal error detected, terminate program
	printf("Error: %s", msg);
	(void)getchar(); // wait for user input to close program after error message
	exit(1);
}

int main(void) {
	long outmode, inmode;

	// get and validate input and output handles
	if ((scrout = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
		terminate("Invalid output handler, can't open output.");
	}

	if ((kbin = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
		terminate("Invalid input handler, can't open input.");
	}

	// set and validate terminal mode for screen output
	if (!GetConsoleMode(scrout, &outmode)) {
		terminate("Can't get console mode");
	}

	outmode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	if (!SetConsoleMode(scrout, outmode)) {
		terminate("Can't set console mode");
	}

	// set and validate terminal mode for screen input
	if (!GetConsoleMode(kbin, &inmode)) {
		terminate("Can't get console mode");
	}
		
	inmode |= ENABLE_VIRTUAL_TERMINAL_INPUT;

	if (!SetConsoleMode(kbin, inmode)) {
		terminate("Can't set console mode");
	}

	// initialize screen and robot
	screen_init();
	robot_init();

	/* Enable keypad escape sequence */
	KPNM

	init_log_file();

	// initialize map for cave input
	cave_map = create_map();

	// enter the main control loop (move robot and modify map)
	go_robot_go();

	printf("Cave map program complete. Press any key to exit.");

	free(cave_map);
	(void)getchar(); // hang until user presses key to exit

	return 0;
}