#define _CRT_SECURE_NO_WARNINGS

/*
The purpose of this module is for opening a secondary console that will log coordinates,
cell attributes, events like portal switching or map saving, and take in inputs in cell editing mode.

As of November 1, 2024, for the Task 5 submission, this code is unused and replaced by
the code in logger.h/c due to errors and time constraints. Notes on the errors encounterd
can be found starting on line 60. However, this multiple console approach will likely be
present in our final solution for this project.
*/

#include <windows.h> // library to let us access the Windows API so we can control consoles programatically, full console docs here: https://learn.microsoft.com/en-us/windows/console/console-reference
#include <stdio.h>

FILE* message_console = NULL; // initialize new file stream for secondary console

DWORD message_console_pid; // storing process ID for the secondary console so we can re-attach to it as needed

int primary_console_active = 1; // store console state

void open_message_console() {

    // create a new console for displaying messages and taking
    FreeConsole(); // this allows us to detach from main default console (in our case, the one for the map display) https://learn.microsoft.com/en-us/windows/console/freeconsole
    AllocConsole(); // now we are free to create a new console with this call https://learn.microsoft.com/en-us/windows/console/allocconsole

	primary_console_active = 0; // secondary console should now be active

	freopen_s(&message_console, "CONOUT$", "w", stderr); // specifying the CONOUT$ value for the console output, https://learn.microsoft.com/en-us/windows/console/setstdhandle

	// get secondary console PID so we can get back to it
	message_console_pid = GetCurrentProcessId(); // docs on process/threads API https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentprocessid
}

void close_message_console() {

    // if the console file stream is open
    if (message_console) {
        fclose(message_console); // close it
    }
    FreeConsole();  // close the secondary console
}

void toggle_input_console(int setPrimary) {
    // check if we want to toggle to primary (map) console
    if (setPrimary) {
		// free process from secondary console and attach back to main
		FreeConsole();
		AttachConsole(ATTACH_PARENT_PROCESS); // using this special param that always uses parent console, AttachConsole docs here: https://learn.microsoft.com/en-us/windows/console/attachconsole 
		primary_console_active = 1;

    }
    else {
        // free process from main console and attach to secondary
		FreeConsole();
		AttachConsole(message_console_pid);
		primary_console_active = 0;

		/*
		This is where we have an error. Once we free the main console again and attach the secondary,
		stdout and stderr may have been reset or something. However, trying freopen_s on the message_console file stream
		again throws an error. And, when printf-ing or fprintf-ing stderr at this step, they do not show up in the main console,
		which is good, means something has happened, but they are not showing up here in secondary console

		Leaving this comment on Nov 1, 2024. We have ran out of time for this initial submission (Task 5), but aim to have this functionality 
		ironed out in our final full solution for the project, as it greatly improves the user experience.

		Likely culprits for the issue:
		 - correct console is not being reattached (issue with PID perhaps?)
		 - stderr is not writing where we want it to write
		*/

		printf("Switched to secondary"); // note how this does not show up anywhere, nor does anything using print_message following the toggle

		//freopen_s(&message_console, "CONOUT$", "w", stderr);
    }
}

void print_message(const char* msg) {
    // print message to the secondary console
	if (primary_console_active) {
		toggle_input_console(0);
	}
    fprintf(stderr, "%s\n", msg);
}

// welcome message that includes an overview of the program and instructions
// for the user, printed to the secondary message console at bootup
void print_welcome_message() {
	print_message("Welcome to the Lunar Ice Cave Explorer (LICE) System!");
	print_message("---------------------------------------------------");
	print_message("This program allows you to create and explore a cave map.");
	print_message("Use the robot to draw and navigate through the cave, reading and setting cell attributes along the way.");
	print_message("");
	print_message("Controls:");
	print_message("  - Arrow Keys: Move the robot (Up, Down, Left, Right)");
	print_message("  - 'C' Key: Set attributes for the current cell you are on (elevation, friction, etc.)");
	print_message("  - Insert Key: Toggle between Movement and Drawing modes");
	print_message("  - Home Key: Exit the program");
	print_message("");
	print_message("Modes:");
	print_message("  - Movement Mode: Move the robot without leaving a trail.");
	print_message("  - Drawing Mode: Leave a tunnel trail as you move.");
	print_message("");
	print_message("Instructions:");
	print_message("  1. Start in Movement Mode to navigate.");
	print_message("  2. Toggle to Drawing Mode to create tunnels.");
	print_message("  3. Press 'C' to set cell attributes at the robot's location.");
	print_message("  4. Use Home to exit when you're finished.");
	print_message("");
	print_message("Enjoy exploring and mapping the lunar caves!");
	print_message("---------------------------------------------------");
}

void print_unrecognized_command_message(char unrecognized_command) {
	char msg[100];

	// since we need to pass a string to print_message for the second console, using sprintf to just store
	// the output string in a char, then passing it to print_message (docs on sprintf: https://www.geeksforgeeks.org/sprintf-in-c/)
	sprintf(msg, "Unrecognized command: '%c'. Please refer to the available commands below.", unrecognized_command);
	print_message(msg);

	// display list of commands and possible modes so user is reminded of what commands to actually use
	print_message("Valid Commands:");
	print_message("  - Arrow Keys: Move the robot (Up, Down, Left, Right)");
	print_message("  - 'C' Key: Set attributes for the current cell (elevation, friction, etc.)");
	print_message("  - Insert Key: Toggle between Movement and Drawing modes");
	print_message("  - Home Key: Exit the program");
	print_message("");
	print_message("Modes:");
	print_message("  - Movement Mode: Move the robot without leaving a trail.");
	print_message("  - Drawing Mode: Leave a tunnel trail as you move.");
	print_message("");
	print_message("Please try again with a valid command.");
}