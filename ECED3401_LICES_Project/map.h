#ifndef MAP_H
#define MAP_H

#define MAP_SIZE 100 // 1km x 1km grid at 10m cells gives 100 x 100 cells
#define MAX_LAYERS 100 // 100 layers, 1m tall each
#define MAX_ACTIVE_LAYERS 2 // max layers to be stored in memory at a time

// structure for a single cell within a layer
typedef struct {
	int elevation;
	int friction;
	int radiation;
	int ritterbarium;
	int ice_percentage;
	char type; // options are R, P, and I,
	char printed_symbol; // storing symbol printed, if any, for map visual purposes
	int isPortal;
	int portalDestinationLayer;
	int unsaved;
} Cell;

// struct for single layer, a 1km x 1km grid, within a complete map
typedef struct {
	Cell cells[MAP_SIZE][MAP_SIZE];
	int initialized;
	int unsaved;
} Layer;

// struct for the entire map, holding up to 100 stacked layers
typedef struct {
	Layer layers[MAX_LAYERS];
	int current_layer; // index of the current layer within the layers array
} Map;

// function to initialize a map with defaul cell values
Map* create_map();

// function to initialize a new layer
void initialize_layer(Map* map, int layer_index);

// function to handle moving through a portal
void handle_portal(Map* map, int from_layer, int to_layer);

// function to draw the map visible in the viewport of the console at its current size
void draw_visible_map(Map* map);

void move_to_layer_zero(Map* map);

void move_to_last_layer_created(Map* map);

#endif // MAP_H
