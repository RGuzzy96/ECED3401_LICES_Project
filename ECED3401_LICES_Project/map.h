#ifndef MAP_H
#define MAP_H

#define MAP_SIZE 100 // 1km x 1km grid at 10m cells gives 100x100 cells
#define MAAX_LAYERS 100 // 100 layers, 1m tall each

typedef struct {
	int elevation;
	int friction;
	int radiation;
	int ritterbarium;
	int ice_percentage;
	char type;
} Cell;
#endif // MAP_H
