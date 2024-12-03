#pragma once

/*

- Header in file with layers
	- Each contains a pointer to the start of its block of the file for the layer map
	- First avail
	- Deleted list

OR, have cells link to each other (don't need this if we store contiguously)

ADD CELL:

- cell addr <- next avail * cell size (memory size) + Header size (or some combo like that)
- Fseek to that location
- new cell.next = previous first cell (layer[L#]
- layer[L#] = new cell
- write cell
- next avail = next avail + 1

*/

#include "map.h"

enum RecStatus { ACTIVE, DELETED };

typedef struct {
	int x_coord;
	int y_coord;
	Cell cell;
	enum RecStatus status;
	long next_cell_addr;
} CellRecord;

typedef struct {
	int layer_index;
	enum RecStatus status;
	long first_cell_addr;
	long first_deleted_addr;
	long next_available_addr;
} LayerHeader;

typedef struct {
	int layer_count;
	long next_available_layer_addr;
	long layer_addresses[MAX_LAYERS];
} FileHeader;

//typedef union {
//	FileHeader file_header;
//	LayerHeader layer_header;
//	CellRecord cell_record;
//} FileRecord;

// functions for file management
void open_and_initialize_file(const char* filename);
void save_layer(int layer_index);
int load_layer(Map *map, int layer_index);
void delete_layer(int layer_index);
void add_cell(int layer_index, Cell new_cell);
void traverse_layer(int layer_index);