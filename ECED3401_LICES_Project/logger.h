#pragma once

#include <stdio.h>

void init_log_file();

void close_log_file();

void log_message(const char* message);

void print_welcome_log();

void print_unrecognized_command(char unrecognized_command);