#define _CRT_SECURE_NO_WARNINGS

#include "file_storage.h"
#include "logger.h"
#include "globals.h"
#include "map.h"
#include "screen.h"

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define LAYER_UNNUSED -1

FILE* file_ptr = NULL;

int open_file(const char* filename) {
	
	// open file in the specified mode
	file_ptr = fopen(filename, "rb+");
	if (!file_ptr) {
		file_ptr = fopen(filename, "wb+");
	}

	// return 1 if opened successfully, 0 if not
	return (file_ptr != NULL);
}

int is_file_initialized() {
	FileHeader header;

	fseek(file_ptr, 0, SEEK_SET);
	fread(&header, sizeof(FileHeader), 1, file_ptr);

	if (header.layer_count >= 0 && header.layer_count < MAX_LAYERS) {
		return 1; // file has a valid header, so is initialized already
	}

	return 0; // not initialized
}

void init_file_header() {
	FileHeader header = { 0 };

	header.layer_count = 0;
	header.next_available_layer_addr = sizeof(FileHeader);

	// initializing the layer addresses to unnused
	for (int i = 0; i < MAX_LAYERS; i++) {
		header.layer_addresses[i] = LAYER_UNNUSED;
	}

	fseek(file_ptr, 0, SEEK_SET);
	fwrite(&header, sizeof(FileHeader), 1, file_ptr);
	fflush(file_ptr);
};

void open_and_initialize_file() {

	char filename[256] = { 0 };
	int index = 0;

	print_msg("Enter file name for map storage: ", 0);

	while (1) {
		char ch = (char)_getch();

		// checking for enter to end filename input
		if (ch == '\r') {
			break;
		}

		if (index < 256) {
			filename[index++] = ch;
		}

		char msg[280];
		sprintf(msg, "Enter file name for map storage: %s", filename);
		print_msg(msg, 0);
	}

	// try to open the file
	if (!open_file(filename)) {
		return;
	}

	if (!is_file_initialized()) {
		init_file_header();
	}
}

FileHeader load_file_header() {
	FileHeader header;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&header, sizeof(FileHeader), 1, file_ptr);
	return header;
}

void save_layer(int layer_index) {
	FileHeader file_header = load_file_header();

	log_message("Trying to save layer");

	int isNew = 0;

	// check if layer exists already
	if (file_header.layer_addresses[layer_index] == LAYER_UNNUSED) {
		log_message("Layer does not exist already");
		
		// initialize layer header
		LayerHeader layer_header = { 0 };
		layer_header.first_cell_addr = -1;
		layer_header.first_deleted_addr = -1;
		layer_header.next_available_addr = file_header.next_available_layer_addr + sizeof(LayerHeader);
		layer_header.layer_index = layer_index;
		layer_header.status = ACTIVE;

		fseek(file_ptr, file_header.next_available_layer_addr, SEEK_SET);
		fwrite(&layer_header, sizeof(LayerHeader), 1, file_ptr);

		// update file header
		file_header.layer_addresses[layer_index] = file_header.next_available_layer_addr;
		file_header.next_available_layer_addr += sizeof(LayerHeader);
		file_header.layer_count++;

		char msg[100];
		sprintf(msg, "This addr and next avail: %u %u", file_header.layer_addresses[layer_index], file_header.next_available_layer_addr);
		log_message(msg);

		fseek(file_ptr, 0, SEEK_SET);
		fwrite(&file_header, sizeof(FileHeader), 1, file_ptr);

		isNew = 1;
	}

	LayerHeader layer_header;
	fseek(file_ptr, file_header.layer_addresses[layer_index], SEEK_SET);
	fread(&layer_header, sizeof(LayerHeader), 1, file_ptr);

	layer_header.next_available_addr = file_header.next_available_layer_addr;

	int last_cell_x = 0;
	int last_cell_y = 0;
	int found_an_unsaved = 0;

	// loop through layer cols
	for (int x = 0; x < MAP_SIZE; x++) {

		// loop through layer rows
		for (int y = 0; y < MAP_SIZE; y++) {
			
			// get pointer to the current cell in memory
			Cell* cell = &cave_map->layers[layer_index].cells[x][y];

			// check if cell has changes that need to be saved
			if (cell->unsaved) {
				// save it
				CellRecord cell_record = {
					.cell = *cell,
					.next_cell_addr = layer_header.next_available_addr + sizeof(CellRecord),
					.status = ACTIVE,
					.x_coord = x,
					.y_coord = y
				};

				// link last cell from previous saves to first cell from this one (only on first to be saved, and if layer has been saved before)
				if (layer_header.first_cell_addr != -1 && found_an_unsaved == 0) {
					long last_cell_addr = layer_header.first_cell_addr;
					CellRecord last_cell_record;
					while (last_cell_addr != -1) {
						fseek(file_ptr, last_cell_addr, SEEK_SET);
						fread(&last_cell_record, sizeof(CellRecord), 1, file_ptr);
						if (last_cell_record.next_cell_addr == -1) {
							last_cell_record.next_cell_addr = layer_header.next_available_addr;
							fseek(file_ptr, last_cell_addr, SEEK_SET);
							fwrite(&last_cell_record, sizeof(CellRecord), 1, file_ptr);
							break;
						}
						last_cell_addr = last_cell_record.next_cell_addr;
					}
				}

				fseek(file_ptr, layer_header.next_available_addr, SEEK_SET);
				fwrite(&cell_record, sizeof(CellRecord), 1, file_ptr);
				
				// if this is the first time saving layer, update first cell address
				if (layer_header.first_cell_addr == -1) {
					layer_header.first_cell_addr = layer_header.next_available_addr;
				}

				layer_header.next_available_addr += sizeof(CellRecord);
				
				
				fseek(file_ptr, file_header.layer_addresses[layer_index], SEEK_SET);
				fwrite(&layer_header, sizeof(LayerHeader), 1, file_ptr);

				// mark cell as saved
				cell->unsaved = 0;				

				found_an_unsaved = 1;
			}

			last_cell_y = y;
		}

		last_cell_x = x;
	}

	// update address of last updated cell to be -1
	CellRecord cell_record = {
		.cell = cave_map->layers[layer_index].cells[last_cell_x][last_cell_y],
		.next_cell_addr = -1,
		.status = ACTIVE,
		.x_coord = last_cell_x,
		.y_coord = last_cell_y
	};

	fseek(file_ptr, layer_header.next_available_addr - sizeof(CellRecord), SEEK_SET);
	fwrite(&cell_record, sizeof(CellRecord), 1, file_ptr);

	file_header.next_available_layer_addr = layer_header.next_available_addr;

	char msg[100];
	sprintf(msg, "Updated next avail after saving: %u", file_header.next_available_layer_addr);
	log_message(msg);

	fseek(file_ptr, 0, SEEK_SET);
	fwrite(&file_header, sizeof(FileHeader), 1, file_ptr);
}

void save() {

	for (int i = 0; i < MAX_LAYERS; i++) {
		Layer* layer = &cave_map->layers[i];
		if (layer->initialized == 1 && layer->unsaved == 1) {
			log_message("Found a layer to save");
			save_layer(i);
		}
	}

	unsaved_changes = 0;

	print_msg(is_drawing_mode ? "Drawing Mode" : "Movement Mode", 1);
}

void manage_layers_in_memory(int curr_index) {
	int evict_index = -1;

	// check if this layer is already in our active layer list
	for (int i = 0; i < MAX_ACTIVE_LAYERS; i++) {
		if (active_layers[i] == curr_index) {
			return;
		}
	}

	// if it is not, check the active layer list and see which is furthest away from this index
	int distance1 = active_layers[0] - curr_index;
	int distance2 = active_layers[1] - curr_index;

	// clear this layer from map memory, and put current index in active layers
}

int load_layer(Map *map, int layer_index) {
	// load the file header to get layer address
	FileHeader header = load_file_header();

	// check if layer has already been saved to file
	if (header.layer_addresses[layer_index] != LAYER_UNNUSED) {		
		log_message("Layer has been saved before");
		// get layer header
		char msg[50];
		sprintf(msg, "Looking for layer at: %u", header.layer_addresses[layer_index]);
		log_message(msg);

		LayerHeader layer_header;
		fseek(file_ptr, header.layer_addresses[layer_index], SEEK_SET);
		fread(&layer_header, sizeof(LayerHeader), 1, file_ptr);

		Layer* current_layer = &map->layers[layer_index];

		long cell_address = layer_header.first_cell_addr;

		char ms[50];
		sprintf(ms, "Looking for first cell at: %u", layer_header.first_cell_addr);
		log_message(ms);

		// track loop count to abort reading cells after max possible encountered for layer
		int loop_count = 0;

		// loop through the cells in the layer that have been saved to file
		while (cell_address != -1 && loop_count < (MAP_SIZE * MAP_SIZE)) {
			CellRecord cell_buffer;
			fseek(file_ptr, cell_address, SEEK_SET);
			fread(&cell_buffer, sizeof(CellRecord), 1, file_ptr);

			current_layer->cells[cell_buffer.x_coord][cell_buffer.y_coord] = cell_buffer.cell;

			cell_address = cell_buffer.next_cell_addr;

			loop_count++;
		}

		return 1;
	}
	else {
		log_message("Layer has not been saved before");
		return 0;
	}
}
