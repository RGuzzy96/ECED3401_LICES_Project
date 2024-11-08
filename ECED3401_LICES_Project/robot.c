#define _CRT_SECURE_NO_WARNINGS

#include "VT100.h"
#include "screen.h"
#include "robot.h"
#include "globals.h"
#include "logger.h"

#define START_SYM	'*'

enum direction { NORTH, SOUTH, EAST, WEST, IDLE };

/* Tunnel symbol to display [new dir][old dir] */
char cell_sym[4][5] = {
/*	Old: NO  SO  ET  WT  IDL    New:*/
		{VR, VR, LR, LL, VR}, /* NO */
		{VR, VR, UR, UL, VR}, /* SO */
		{UL, LL, HR, HR, HR}, /* ET */
		{UR, LR, HR, HR, HR}  /* WT */
};

short del_y[] = { -1, 1, 0, 0 };
short del_x[] = { 0, 0, 1, -1 };

ROBOT mylice;

/* ASCII-to-DEC graphic characters */
enum symbol asc_dec[] = {
	'j', 'k', 'l', 'm', 'n', 'q', 't', 'u', 'v', 'w', 'x', ' ' };

void robot_init()
{
/*
 - Initialize robot to center of screen
 - Show robot at center of screen
*/
mylice.x = screen.center.col;
mylice.y = screen.center.row;
mylice.curr_dir = IDLE;

printf(CSI "%dm" CSI "%dm", BGGREEN, FGWHITE);

draw_object(mylice.x, mylice.y, START_SYM);
}

void robot_move(char ch)
{
enum direction new_dir;
enum symbol new_off; /* Offset into asc_dec[] */
char new_sym;	/* Symbol from asc_dec[] */
int new_x;
int new_y;

/* DEC Keypad sequence (U, D, R, L) */
new_x = mylice.x + del_x[ch - 'A'];
new_y = mylice.y + del_y[ch - 'A'];

/* Should check for edge of world */

/* New direction? */
if (new_y < mylice.y)
	new_dir = NORTH;
else
if (new_y > mylice.y)
	new_dir = SOUTH;
else
if (new_x < mylice.x)
	new_dir = WEST;
else
	new_dir = EAST;

Cell* current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x][mylice.y];

/* Get symbol offset */
new_off = cell_sym[new_dir][mylice.curr_dir];
new_sym = current_cell->isPortal ? 'O' : asc_dec[new_off];

/* Draw symbol if possible */

EDLDM /* Enable DEC line drawing mode */

if (is_drawing_mode) {
	if (draw_object(mylice.x, mylice.y, new_sym) == 0)
	{
		/* Draw worked */

		// store the symbol we are drawing in this cell
		current_cell->printed_symbol = new_sym;

		/* Remember last valid position */
		mylice.oldx = mylice.x;
		mylice.oldy = mylice.y;
		mylice.old_dir = mylice.curr_dir; 
		mylice.curr_dir = new_dir;
		/* Possible overshoot to invalid position */
		mylice.x = new_x;
		mylice.y = new_y;
	}
	else
	{
		/* Restore robot to last valid position */
		mylice.x = mylice.oldx;
		mylice.y = mylice.oldy;
		mylice.curr_dir = mylice.old_dir;
	}

}
else {
	// in movement mode, move robot without drawing
	mylice.oldx = mylice.x;
	mylice.oldy = mylice.y;
	mylice.old_dir = mylice.curr_dir;
	mylice.curr_dir = new_dir;
	mylice.x = new_x;
	mylice.y = new_y;

	// replace previous robot symbol with previous cell symbol
	char prev_sym = cave_map->layers[cave_map->current_layer].cells[mylice.oldx][mylice.oldy].printed_symbol;
	if (prev_sym != ' ') {
		draw_object(mylice.oldx, mylice.oldy, prev_sym);
	}
	else {
		printf(CSI "%dm", BGBLACK);
		draw_object(mylice.oldx, mylice.oldy, ' ');
		printf(CSI "%dm", BGGREEN);
	}
	
	// draw the robot symbol in the new position
	draw_object(mylice.x, mylice.y, START_SYM);
}

// log new coordinates
char msg[200];
if (current_cell->isPortal) {
	sprintf(msg, "Take portal from Layer %d to Layer %d? Press 'p' to confirm", current_cell->elevation, current_cell->portalDestinationLayer);
}
else {
	sprintf(msg, is_drawing_mode ? "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%) - DRAWING" 
		: "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%)", 
		mylice.x, 
		mylice.y, 
		cave_map->current_layer,
		current_cell->elevation,
		current_cell->friction,
		current_cell->radiation,
		current_cell->ritterbarium,
		current_cell->type,
		current_cell->ice_percentage
	);
}

print_msg(msg, 0);
log_message(msg);

EAM /* Enable ASCII mode */
}

void create_portal(char portal_direction) {
	Cell* current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x][mylice.y];
	
	current_cell->isPortal = 1;
	current_cell->printed_symbol = 'O';
	current_cell->portalDestinationLayer = portal_direction == 'd' ? cave_map->current_layer - 1 : cave_map->current_layer + 1;
	draw_object(mylice.x, mylice.y, 'O');
	log_message("Portal created");
}

void use_portal() {
	Cell* current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x][mylice.y];

	if (current_cell->isPortal) {
		int from_layer = cave_map->current_layer;
		int to_layer = current_cell->portalDestinationLayer;

		char msg[50];
		sprintf(msg, "Taking portal to Layer %d from Layer %d", to_layer, from_layer);
		print_msg(msg, 0);
		handle_portal(cave_map, from_layer, to_layer);

		Cell* new_portal_cell = &cave_map->layers[to_layer].cells[mylice.x][mylice.y];

		// create portal connection in new layer if it does not already exist
		if (!new_portal_cell->isPortal) {
			char portal_direction = to_layer > from_layer ? 'd' : 'u';
			create_portal(portal_direction);
		}
	}
	else {
		print_msg("This cell is not a portal. Switch to drawing mode and hit 'p' to make it one.", 0);
	}	
}

int isValidAttribute(char att) {
	return (att == 'f' || att == 'r' || att == 'b' || att == 't' || att == 'i');
}

int isValidAttVal(char att, int val) {
	int isValid = 0;

	switch (att) {
	case 'f':
		isValid = (val >= 1 && val <= 10);
		break;
	case 'r': 
		isValid = (val >= 0 && val <= 100);
		break;
	case 'b':
		isValid = (val >= 0 && val <= 100);
		break;
	case 'i': 
		isValid = (val >= 0 && val <= 100);
		break;
	default:
		isValid = 0;
		break;
	}

	return isValid;
}

int isValidType(char type) {
	return (type == 'R' || type == 'I' || type == 'S');
}

void set_cell_attributes() {
	Cell* current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x][mylice.y];
	char attribute;
	char value[10];
	int intValue;

	print_msg("Editing cell attributes. Refer to run_logs.txt for detailed instructions. Press 'q' to exit.", 1);

	while (1) {
		char msg[100];
		sprintf(msg, "(F: %d, R: %d, B: %d, T: %c, I: %d%)",
			current_cell->friction,
			current_cell->radiation,
			current_cell->ritterbarium,
			current_cell->type,
			current_cell->ice_percentage
		);
		print_msg(msg, 0);

		attribute = (char)_getch();

		// handle quit editing mode
		if (attribute == 'q') {
			break;
		}

		if (isValidAttribute(attribute)) {
			print_msg("Enter new value", 0);

			int index = 0;

			while (1) {
				char ch = (char)_getch();
				if (ch == 'q') {
					break;
				}

				if (ch == '\r') {  // user hit enter key to end input
					value[index] = '\0';
					break;
				}

				char valMsg[50];

				if (isdigit(ch) || isValidType(ch)) {
					value[index++] = ch;
				}
			}

			int isValidInput = 0;
			if (attribute == 't') {
				if (isValidType(value)) {
					isValidInput = 1;
				}
			}
			else {
				intValue = atoi(value);
				if (isValidAttVal(attribute, intValue)) {
					isValidInput = 1;
				}
			}

			if (!isValidInput) {
				print_msg("Invalid input! Press 'r' to retry or 'q' to quit", 0);
				while (1) {
					char ch = (char)_getch();
					if (ch == 'r' || ch == 'q') {
						break;
					}
				}
			}
			else {
				switch (attribute) {
					case 'f':
						current_cell->friction = intValue;
						break;
					case 'r':
						current_cell->radiation = intValue;
						break;
					case 'b':
						current_cell->ritterbarium = intValue;
						break;
					case 't':
						current_cell->type = value;
						break;
					case 'i':
						current_cell->ice_percentage = intValue;
						break;
				}
			}
		}
	}

	char msg[200];
	sprintf(msg, is_drawing_mode ? "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%) - DRAWING"
		: "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%)",
		mylice.x,
		mylice.y,
		cave_map->current_layer,
		current_cell->elevation,
		current_cell->friction,
		current_cell->radiation,
		current_cell->ritterbarium,
		current_cell->type,
		current_cell->ice_percentage
	);
	print_msg(msg, 0);

	print_msg(is_drawing_mode ? "Drawing Mode" : "Movement Mode", 1);
}