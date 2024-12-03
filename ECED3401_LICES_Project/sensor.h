#pragma once

// this is the module that will control the licer's sensor

// the sensor looks at cells around it and their attributes to decide on actions and paths

// use dynamic programming? maybe just creates an ExploredMap Map ?

/*

- checks through recursively
	- we could run into errors where the path is a loop
		- keep an already explored state (or just has value or not)

- are creating a new map? or just updating a visited state on existing map
	- probably the latter

- we also need battery state
	- will go down when we move
		- or take actions, and different for friction cells, etc
	- up when we replace

LICER -> read_sensor() -> take actions -> builds map -> move -> repeat

Upgrade gradually
 - start to handle it on one layer

*/