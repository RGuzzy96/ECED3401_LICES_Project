#pragma once

/*

Not currently used as of November 1, 2024, see note in message_console.c

*/

#include <stdio.h>

// function to open a secondary console window for messages and secondary input (setting cell characteristics, for example)
void open_message_console();

// function to close the secondary console window
void close_message_console();

// function to redirect stdin to the secondary console
void toggle_input_console(int setPrimary);

// function to print a message to the secondary console
void print_message(const char* msg);

// function that prints welcome welcome message that includes an overview of the program and instructions
// for the user, printed to the secondary message console at bootup
void print_welcome_message();

// function to print when the user enters an unrecognized command
void print_unrecognized_command_message(char unrecognized_command);