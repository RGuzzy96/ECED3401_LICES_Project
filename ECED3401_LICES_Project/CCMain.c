/*
 - Cursor control and drawing example
 - Accepts cursor control (U, D, R, L) to move "robot"
 - ECED 3401
	- 3 Oct 24 - Original (Dr. Larry Hughes)
	- 1 Nov 24 - Modified for Task 5 implementation (Ryan Guzzwell, Hemang Arora)

 - See:
	- https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
*/

#include "VT100.h"
#include "Screen.h"
#include "Robot.h"

HANDLE scrout;	/* Output stream */
HANDLE kbin;	/* Input stream */

void terminate(char* msg)
{
/* Fatal error detected - terminate */
printf("Error: %s\n", msg);
(void)getchar();
exit(1);
}

main()
{
/*
 - Setup Main screen based on window size
 - initialize screen and robot
 - run robot until Home key pressed
*/
long outmode, inmode;

/* Get input and output handles: */
if ((scrout = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	terminate("Can't open output");

if ((kbin = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
	terminate("Can't open input");

/* Set terminal mode for screen input: */
if (!GetConsoleMode(scrout, &outmode))
	terminate("Can't get old console mode");

outmode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
if (!SetConsoleMode(scrout, outmode))
	terminate("Can't set console mode");

/* Set terminal mode for screen input: */
if (!GetConsoleMode(kbin, &inmode))
	terminate("Can't get old console mode");

inmode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
if (!SetConsoleMode(kbin, inmode))
	terminate("Can't set console mode");

/* Switch to VT-100 mode */
screen_init();

/* Enable robot */
robot_init();

/* Enable keypad escape sequence */
KPNM

/* Check robot for movement */
go_robot_go();

print_msg("Done!");

(void) getchar();

}
