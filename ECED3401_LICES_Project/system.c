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

// function to flush the input buffer - needed to prevent the issue of user holding down keys for a long time
// and then stopping and noticing that the program seems to loop, but really it is just addressing the long
// list of characters and needs some time
void flush_input_buffer() {
	while (_kbhit()) {
		_getch();
	}
}

void go_robot_go()
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
			
				// check for robot movement or other escape commands
				switch (ch)
				{
				case 'H':
					done = TRUE;
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
			default:
				print_msg("Uncrecognized command", 0);
				print_unrecognized_command(ch);
				break;
			}
		}
		}
	}

	

}