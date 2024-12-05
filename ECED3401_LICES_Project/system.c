/*
 - Reads kb
 - Decides on action:
	- Robot move
	- Screen move

 - ESC sequences:
	- [ + H - home
	- [ + A|B|C|D - U, D, L, R

 - ECED 3401
	- 3 Oct 24 - Original (Dr. Larry Hughes)
	- 1 Nov 24 - Modified for Task 5 implementation (Ryan Guzzwell, Hemang Arora)

 - Source:
	- https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#input-sequences
*/

#include "VT100.h"
#include "robot.h"
#include "map.h"
#include "logger.h"
#include "globals.h"
#include "screen.h"
#include "file_storage.h"
#include "time.h"

#define EMULATOR_STEP_DEBOUNCE 200

clock_t last_emulator_step_time = 0;

// function to flush the input buffer - needed to prevent the issue of user holding down keys for a long time
// and then stopping and noticing that the program seems to loop, but really it is just addressing the long
// list of characters and needs some time
void flush_input_buffer() {
	while (_kbhit()) {
		_getch();
	}
}

void select_run_mode() {
	print_msg("Select run mode: d = Design, e = Emulator", 1);

	int done;

	done = FALSE;

	while (!done) {
		if (_kbhit()) {
			char ch = (char)_getch();
			char msg[25];
			sprintf(msg, "Ch: %c", ch);
			log_message(msg);
			switch (ch) {	
			case 'd':
				run_mode = DESIGN;
				done = TRUE;
				break;
			case 'e':
				run_mode = EMULATOR;
				done = TRUE;
				break;
			}
		}	
	}
}

void design_loop()
{
int done;
char ch;
Cell cell;

done = FALSE;

print_msg(is_drawing_mode ? "Drawing Mode" : "Movement Mode", 1);

while (!done)
{
	if (check_screen_size()) {

	}

	if (_kbhit()) {
		ch = (char)_getch();
		int offset_x = 0; // to offset robot movement when resizing in x direction
		int offset_y = 0; // same but in y direction
	
		if (ch == ESCAPE)
		{
			if ((char)_getch() == '[')
			{
				ch = (char)_getch();

				// checking for ctrl + arrow key combo for map viewport movement
				if (ch == '1' && (char)_getch() == ';') {
					char ctrl_code = (char)_getch();

					if (ctrl_code == '5') {
						switch (ch = (char)_getch()) {
						case 'A':
							viewport_y--;
							offset_y = 1;
							draw_visible_map(cave_map);
							flush_input_buffer();
							break;
						case 'B':
							viewport_y++;
							offset_y = -1;
							draw_visible_map(cave_map);
							flush_input_buffer();
							break;
						case 'C':
							viewport_x++;
							offset_x = -1;
							draw_visible_map(cave_map);
							flush_input_buffer();
							break;
						case 'D':
							viewport_x--;
							offset_x = 1;
							draw_visible_map(cave_map);
							flush_input_buffer();
							break;
						}
					}
				}

				char msg[25];
				sprintf(msg, "Ch: %c", ch);
				log_message(msg);
			
				// check for robot movement or other escape commands
				switch (ch)
				{
				case 'H':
					if (is_drawing_mode) {
						move_to_layer_zero(cave_map);
					}
					else {
						// move cursor to 1, 1
					}
					break;
				case 'F':
					if (!is_drawing_mode) {
						done = TRUE;
						quit_program = 1;
					}
					else {
						move_to_last_layer_created(cave_map);
					}
					break;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
					robot_move(ch, offset_x, offset_y);
					break;
				case '2':
					if ((char)_getch() == '~') {
						is_drawing_mode = !is_drawing_mode; // toggle mode	
						log_message(is_drawing_mode ? "Drawing Mode activated" : "Movement Mode activated");
						print_msg(is_drawing_mode ? "Drawing Mode" : "Movement Mode", 1);
					}
					break;
				}
			}
		}
		else {
			// no escape commands, handling regular utility commands
			switch (ch) {
			case 'c':
				set_cell_attributes();
				break;
			case 'p':
				if (is_drawing_mode) {
					char portal_direction;
					if (cave_map->current_layer != 0) {
						print_msg("What direction should this portal go? Hit 'u' for up and 'd' for down a layer", 0);
						while (1) {
							portal_direction = (char)_getch();
							if (portal_direction == 'u' || portal_direction == 'd') {
								break;
							}
						}
					}
					else {
						portal_direction = 'u';
					}
					create_portal(portal_direction);
				}
				else {
					use_portal();
				}
				break;
			case 's':
				log_message("Trying to save map layer!");
				save();
				break;
			case 'm':
				log_message("Changing mode");
				select_run_mode();
				done = TRUE;
				break;
			default:
				print_msg("Uncrecognized command", 0);
				print_unrecognized_command(ch);
				break;
			}
		}
		}
	}
}

void reset_emulator() {
	// reset to first layer to start emulation
	if (cave_map->current_layer != 0) {
		cave_map->current_layer = 0;
	}

	draw_visible_map(cave_map);
	robot_init();

	// reset visited list to no cells visited
	memset(visited_list, 0, sizeof(visited_list));
}

void emulator_loop() {
	log_message("Starting emulator...");

	print_msg("Press 's' to start emulator", 1);
	print_msg("Coordinates, cell attributes, and other details will show up here", 0);

	int done = FALSE;
	int emulator_running = 0;
	int found_ice = 0;
	int already_printed_ice_message = 0;

	// reset to first layer to start emulation
	if (cave_map->current_layer != 0) {
		cave_map->current_layer = 0;
	}

	draw_visible_map(cave_map);
	robot_init();

	// reset visited list to no cells visited
	memset(visited_list, 0, sizeof(visited_list));

	while (!done) {

		if (_kbhit()) {
			char ch = (char)_getch();

			switch (ch) {
			case 'm':
				log_message("Changing mode");
				select_run_mode();
				done = TRUE;
				break;
			case 's':
				if (!emulator_running) {
					log_message("Starting emulator");
					print_msg("Emulator running. Press 'p' to pause", 1);
					emulator_running = 1;

					// if we are running again after finding ice
					if (found_ice) {
						found_ice = 0;
						already_printed_ice_message = 0;

						reset_emulator();
					}
				}
				break;
			case 'p':
				log_message("Pausing emulator");
				emulator_running = 0;
				print_msg("Emulator paused. Press 's' to start, 'r' to reset, or 'q' to quit program", 1);
				break;
			case 'r':
				// reset the robot state here
				log_message("Resetting emulator");
				emulator_running = 0;
				found_ice = 0;
				already_printed_ice_message = 0;
				reset_emulator();
				print_msg("Emulator reset. Press 's' to start", 1);
				break;
			case 'q':
				log_message("Quitting program");
				done = TRUE;
				quit_program = 1;
				break;
			}
		}

		if (emulator_running && !found_ice) {
			clock_t current_time = clock();
			if ((current_time - last_emulator_step_time) * 1000 / CLOCKS_PER_SEC > EMULATOR_STEP_DEBOUNCE) {
				last_emulator_step_time = current_time;
				found_ice = handle_emulator_step();
			}
		}

		// hang program once we have found ice
		if (found_ice && !already_printed_ice_message) {
			print_msg("We have found ice! You can run the emulator again, or switch to design mode to update the map", 1);
			already_printed_ice_message = 1;
		}
	}
}