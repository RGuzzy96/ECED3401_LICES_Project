#define _CRT_SECURE_NO_WARNINGS

#include "VT100.h"
#include "screen.h"
#include "robot.h"
#include "globals.h"
#include "logger.h"
#include "time.h"

#define START_SYM	'*'

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
enum symbol asc_dec[] = { 'j', 'k', 'l', 'm', 'n', 'q', 't', 'u', 'v', 'w', 'x', ' ' };

char determine_new_symbol(char prev_sym, enum direction new_dir) {
	if (prev_sym != ' ' && prev_sym != NUL) {
		switch (new_dir) {
		case NORTH:
			switch (prev_sym) {
			case 'q': return 'n'; // see https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#designate-character-set
			case 'v': return 'n'; // for table outlining the ASCII to DEC line drawing conversion
			case 'u': return 'n';
			case 'l': return 't';
			case 'j': return 'x';
			case 'k': return 'w';
			case 'm': return 'l';
			case 'n': return 'n';
			case 't': return 't';
			case 'w': return 'n';
			}
			break;

		case SOUTH:
			switch (prev_sym) {
			case 'q': return 'n';
			case 'w': return 'n';
			case 'u': return 't';
			case 'l': return 'x';
			case 'j': return 'v';
			case 'k': return 't';
			case 'm': return 'v';
			case 'n': return 'n';
			case 't': return 'n';
			}
			break;

		case EAST:
			switch (prev_sym) {
			case 'x': return 'n';
			case 't': return 'n';
			case 'l': return 'w';
			case 'j': return 'u';
			case 'k': return 'w';
			case 'm': return 't';
			case 'v': return 'u';
			case 'n': return 'n';
			case 'w': return 'w';
			}
			break;

		case WEST:
			switch (prev_sym) {
			case 'x': return 'n';
			case 'u': return 'n';
			case 'j': return 'v';
			case 'k': return 't';
			case 'l': return 'v';
			case 'm': return 't';
			case 'n': return 'n';
			case 't': return 't';
			case 'w': return 'n';
			}
			break;
		}
	}
	return prev_sym;
}

void robot_init()
{
/*
 - Initialize robot to center of screen
 - Show robot at center of screen
*/
mylice.x = screen.center.col;
mylice.y = screen.center.row;
mylice.curr_dir = IDLE;

mylice.battery_level = 1000;

printf(CSI "%dm" CSI "%dm", BGGREEN, FGWHITE);

draw_object(mylice.x, mylice.y, START_SYM);
}

void robot_move(char ch, int offset_x, int offset_y)
{
enum direction new_dir;
enum symbol new_off; /* Offset into asc_dec[] */
char new_sym;	/* Symbol from asc_dec[] */
int new_x;
int new_y;

/* DEC Keypad sequence (U, D, R, L) */
new_x = mylice.x + del_x[ch - 'A'] + viewport_x;
new_y = mylice.y + del_y[ch - 'A'] + viewport_y;

/* New direction? */
if (new_y < mylice.y + viewport_y)
	new_dir = NORTH;
else
if (new_y > mylice.y + viewport_y)
	new_dir = SOUTH;
else
if (new_x < mylice.x + viewport_x)
	new_dir = WEST;
else
	new_dir = EAST;

// make sure robot stays in bounds
if (new_x < 0 || new_x >= MAP_SIZE || new_y < 0 || new_y >= MAP_SIZE) {
	print_msg("Blocked, edge of the map.", 0);
	if (offset_x != 0 || offset_y != 0) {
		if (new_dir == NORTH) {
			viewport_y++;
		} else if (new_dir == SOUTH) {
			viewport_y--;
		} else if (new_dir == WEST) {
			viewport_x++;
		} else if (new_dir == EAST) {
			viewport_x--;
		}
	}
	
	return;
}

// block the robot at the visible edge of the console if not trying to resize
if (!offset_x && !offset_y) {
	if ((mylice.x <= screen.min.col + 1 && del_x[ch - 'A'] < 0) ||
		(mylice.x >= screen.max.col - 2 && del_x[ch - 'A'] > 0)) {
		print_msg("Blocked, reached console edge horizontally. Press 'ctrl' + arrow key to move the map", 0);
		return;
	}

	if ((mylice.y <= screen.min.row + 1 && del_y[ch - 'A'] < 0) ||
		(mylice.y >= screen.max.row - 2 && del_y[ch - 'A'] > 0)) {
		print_msg("Blocked, reached console edge vertically. Press 'ctrl' + arrow key to move the map", 0);
		return;
	}
}

Cell* current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x + viewport_x][mylice.y + viewport_y];

/* Get symbol offset */
new_off = cell_sym[new_dir][mylice.curr_dir];
new_sym = current_cell->isPortal ? 'O' : asc_dec[new_off];

/* Draw symbol if possible */

EDLDM /* Enable DEC line drawing mode */

if (is_drawing_mode && run_mode != EMULATOR) {
	char prev_sym = current_cell->printed_symbol;
	if (prev_sym != ' ' && !current_cell->isPortal) {
		new_sym = determine_new_symbol(prev_sym, new_dir);
	}

	if (draw_object(mylice.x + offset_x, mylice.y + offset_y, new_sym) == 0)
	{
		/* Draw worked */

		// store the symbol we are drawing in this cell
		current_cell->printed_symbol = new_sym;

		// mark this cell as having unsaved changes
		current_cell->unsaved = 1;

		// mark current layer as having unsaved changes
		cave_map->layers[cave_map->current_layer].unsaved = 1;

		// update global unsaved changes state to true
		unsaved_changes = 1;

		/* Remember last valid position */
		mylice.oldx = mylice.x + offset_x;
		mylice.oldy = mylice.y + offset_y;
		mylice.old_dir = mylice.curr_dir; 
		mylice.curr_dir = new_dir;
		/* Possible overshoot to invalid position */
		mylice.x = new_x - viewport_x + offset_x;
		mylice.y = new_y - viewport_y + offset_y;
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
	mylice.oldx = mylice.x + offset_x;
	mylice.oldy = mylice.y + offset_y;
	mylice.old_dir = mylice.curr_dir;
	mylice.curr_dir = new_dir;
	mylice.x = new_x - viewport_x + offset_x;
	mylice.y = new_y - viewport_y + offset_y;

	// replace previous robot symbol with previous cell symbol
	char prev_sym = cave_map->layers[cave_map->current_layer].cells[mylice.oldx + viewport_x][mylice.oldy + viewport_y].printed_symbol;
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

EAM /* Enable ASCII mode */

// get current cell with new coordinates
current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x + viewport_x][mylice.y + viewport_y];

// consume battery due to friction
mylice.battery_level -= current_cell->friction;

// charge up battery from ritterbarium


// log new coordinates
char msg[200];
if (current_cell->isPortal) {
	sprintf(msg, "Take portal from Layer %d to Layer %d? Press 'p' to confirm", current_cell->elevation, current_cell->portalDestinationLayer);
}
else if (run_mode == EMULATOR) {
	sprintf(msg, "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%) - LICER: (BA: %d, DCUs: N/A, BR: N/A)",
		mylice.x + viewport_x,
		mylice.y + viewport_y,
		cave_map->current_layer,
		current_cell->elevation,
		current_cell->friction,
		current_cell->radiation,
		current_cell->ritterbarium,
		current_cell->type,
		current_cell->ice_percentage,
		mylice.battery_level
	);
}
else {
	sprintf(msg, is_drawing_mode ? "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%) - DRAWING"
		: "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%)",
		mylice.x + viewport_x,
		mylice.y + viewport_y,
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
if (unsaved_changes) {
	print_msg(is_drawing_mode ? "Drawing Mode - UNSAVED CHANGES" : "Movement Mode - UNSAVED CHANGES", 1);
}
log_message(msg);
}

void create_portal(char portal_direction) {
	Cell* current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x + viewport_x][mylice.y + viewport_y];
	
	current_cell->isPortal = 1;
	current_cell->printed_symbol = 'O';
	current_cell->portalDestinationLayer = portal_direction == 'd' ? cave_map->current_layer - 1 : cave_map->current_layer + 1;
	current_cell->unsaved = 1;

	cave_map->layers[cave_map->current_layer].unsaved = 1;

	unsaved_changes = 1;

	draw_object(mylice.x, mylice.y, 'O');
	log_message("Portal created");
}

void use_portal() {
	Cell* current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x + viewport_x][mylice.y + viewport_y];

	if (current_cell->isPortal) {
		int from_layer = cave_map->current_layer;
		int to_layer = current_cell->portalDestinationLayer;

		char msg[50];
		sprintf(msg, "Taking portal to Layer %d from Layer %d", to_layer, from_layer);
		print_msg(msg, 0);
		handle_portal(cave_map, from_layer, to_layer);

		Cell* new_portal_cell = &cave_map->layers[to_layer].cells[mylice.x + viewport_x][mylice.y + viewport_y];

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
	Cell* current_cell = &cave_map->layers[cave_map->current_layer].cells[mylice.x + viewport_x][mylice.y + viewport_y];
	char attribute;
	char value[10];
	int intValue;
	int changes_happened = 0;

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
				changes_happened = 1;
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

	if (changes_happened) {
		current_cell->unsaved = 1;
		cave_map->layers[cave_map->current_layer].unsaved = 1;
		unsaved_changes = 1;
	}

	char msg[200];
	sprintf(msg, is_drawing_mode ? "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%) - DRAWING"
		: "(X: %d, Y: %d, Z: %d) (E: %d, F: %d, R: %d, B: %d, T: %c, I: %d%)",
		mylice.x + viewport_x,
		mylice.y + viewport_y,
		cave_map->current_layer,
		current_cell->elevation,
		current_cell->friction,
		current_cell->radiation,
		current_cell->ritterbarium,
		current_cell->type,
		current_cell->ice_percentage
	);


	print_msg(msg, 0);

	if (unsaved_changes) {
		print_msg(is_drawing_mode ? "Drawing Mode - UNSAVED CHANGES" : "Movement Mode - UNSAVED CHANGES", 1);
	}
	else {
		print_msg(is_drawing_mode ? "Drawing Mode" : "Movement Mode", 1);
	}
}

void get_adjacent_cells(Layer* layer, int x, int y, Cell* adjacent_cells[4]) {
	adjacent_cells[NORTH] = &layer->cells[x][y - 1];
	adjacent_cells[SOUTH] = &layer->cells[x][y + 1];
	adjacent_cells[EAST] = &layer->cells[x - 1][y];
	adjacent_cells[WEST] = &layer->cells[x + 1][y];
}

int select_best_cell(Cell* cells[4], int* found_ice, int curr_x, int curr_y) {
	int best_score = 0;
	int best_cell_index = 0;

	for (int i = 0; i < 4; i++) {
		int cell_score = 0;
		int y_offset[4] = { -1, 1, 0, 0 };
		int x_offset[4] = { 0, 0, -1, 1 };

		// check to see if this cell has been drawn in the map
		if (cells[i]->printed_symbol != ' ') {

			// default cell score for not being empty
			cell_score = 5;

			// check for ice
			if (cells[i]->ice_percentage == 100) {
				*found_ice = 1;
				best_cell_index = i;
				break;
			}

			// check if cell has been visited
			clock_t visit_time = visited_list[cave_map->current_layer][curr_x + x_offset[i]][curr_y + y_offset[i]];

			if (visit_time != 0) {
				// get current time and get time since visited
				clock_t current_time = clock();
				double time_diff = (double)(current_time - visit_time) / CLOCKS_PER_SEC;
				
				// apply weighted penalty based on how long ago it was visited
				int penalty = (int)(10 - time_diff);

				// give light penalty to cells encountered long ago (more than random amount added after this too)
				if (penalty < 3) {
					penalty = 3;
				}
				
				cell_score -= penalty;
			}

			// adding small random element to break ties, will add from 0 to 2 points
			cell_score += rand() % (3);

			// add weighting for other factors like radiation and friction
		}

		if (cell_score > best_score) {
			best_score = cell_score;
			best_cell_index = i;
		}
	}

	// return -1 if no cell was chosen as next
	if (!best_score && *found_ice != 1) {
		return -1;
	}

	return best_cell_index;
}

int handle_emulator_step() {

	int offset_x = 0; // work with these and viewport later for automatic map shifting when going out of bounds
	int offset_y = 0;

	Layer* current_layer = &cave_map->layers[cave_map->current_layer];

	Cell* current_cell = &current_layer->cells[mylice.x + viewport_x][mylice.y + viewport_y];
	
	Cell* adjacent_cells[4];

	get_adjacent_cells(current_layer, mylice.x + viewport_x, mylice.y + viewport_y, adjacent_cells);

	// check if we are on a portal
	if (current_cell->isPortal) {
		int visited_count = 0;
		int valid_adjacent_cell_count = 0;
		int y_offset[4] = { -1, 1, 0, 0 };
		int x_offset[4] = { 0, 0, -1, 1 };

		// loop through adjacent cells to see if we have already checked out that route
		for (int i = 0; i < 4; i++) {
			if (adjacent_cells[i]->printed_symbol != ' ') {
				valid_adjacent_cell_count++;

				clock_t visited_time = visited_list[cave_map->current_layer][mylice.x + viewport_x + x_offset[i]][mylice.y + viewport_y + y_offset[i]];
				if (visited_time != 0) {
					visited_count++;
				}
			}
		}

		// if we have visited all valid surrounding cells, lets try our luck through the portal
		if (visited_count == valid_adjacent_cell_count) {
			log_message("All surrounding cells have been visited, using portal");
			use_portal();
			return 0;
		}
		else {
			log_message("Surrounding cells have not been explored, not using portal yet");
		}
	}

	int found_ice = 0;
	int next_cell_index = select_best_cell(adjacent_cells, &found_ice, mylice.x + viewport_x, mylice.y + viewport_y);

	char ch_directions[4] = { 'A', 'B', 'D', 'C' }; // U, D, L, R

	char cell_msg[50];

	if (next_cell_index != -1) {
		robot_move(ch_directions[next_cell_index], offset_x, offset_y);

		// get and store timestamp of visiting cell
		clock_t current_time = clock();
		visited_list[cave_map->current_layer][mylice.x + viewport_x][mylice.y + viewport_y] = current_time;

		if (found_ice) {
			log_message("Oh bingo we got ice!");
			return 1;
		}
	}
	else {
		// handle no best cell chosen
	}

	return 0;
}