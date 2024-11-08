#define _CRT_SECURE_NO_WARNINGS

#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "screen.h"
#include "VT100.h"

Map* create_map() {
	Map* map = (Map*)calloc(1, sizeof(Map)); // initialize map with calloc (zero all values) for size of Map type

	if (map == NULL) {
		fprintf(stderr, "Memory allocation failed for Map\n");
		return NULL;
	}

	// initialize starting layer as top layer (index grows as we go deeper)
	map->current_layer = 0;

	initialize_layer(map, 0);

	return map;
}

void initialize_layer(Map* map, int layer_index) {
	// loop through each col in layer
	for (int x = 0; x < MAP_SIZE; x++) {

		// loop through each cell in col
		for (int y = 0; y < MAP_SIZE; y++) {

			// initialize cell attributes with default values
			map->layers[layer_index].cells[x][y].elevation = layer_index;
			map->layers[layer_index].cells[x][y].friction = 5;
			map->layers[layer_index].cells[x][y].ice_percentage = 0;
			map->layers[layer_index].cells[x][y].radiation = 0;
			map->layers[layer_index].cells[x][y].ritterbarium = 0;
			map->layers[layer_index].cells[x][y].type = 'R'; // default cell type is Rock
			map->layers[layer_index].cells[x][y].printed_symbol = ' ';
		}
	}

	map->layers[layer_index].initialized = 1;
}

void load_layer(Map* map) {
	Layer* current_layer = &map->layers[map->current_layer];
	
	// enable DEC line drawing mode
	EDLDM

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {
			char sym = current_layer->cells[x][y].printed_symbol;
			if (sym != ' ') {
				printf(CSI "%dm", BGGREEN);
			}
			else {
				printf(CSI "%dm", BGBLACK);
			}
			draw_object(x, y, sym);
		}
	}

	// enable ASCII mode
	EAM
}

void handle_portal(Map* map, int from_layer, int to_layer) {
	if (from_layer >= 0 && to_layer <= MAX_LAYERS) {
		map->current_layer = to_layer;

		if (!map->layers[to_layer].initialized) {
			initialize_layer(map, to_layer);
		}
		load_layer(map);

		char msg[50];
		sprintf(msg,"Moved to layer: %d", to_layer);
		log_message(msg);
		print_msg(msg, 0);
	}
	else {
		print_msg("Invalid layer, cannot move through portal.", 0);
	}
}

