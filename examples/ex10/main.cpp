/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Example 10: Not really an example yet, playing with getting the ECS working.
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"

// We're using a stringstream to build the hello world message.
#include <sstream>

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
using namespace rltk::colors;

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	// In this case, we just want to print "Hello World" in white on black.
	if (console->dirty) {
		console->clear();
		console->print(1,1,"Hello World", WHITE, BLACK);
	}
}

struct position_component {
	int x, y;
};

struct position_component2 {
	int x, y;
};

// Your main function
int main()
{
	// Initialize the library. Here, we are providing plenty of into so you can see what is
	// available. There's also the option to use config_simple_px to configure by pixels
	// rather than characters.
	// The first parameter is the path to the font files.
	// The second and third parameters specify the desired console size in screen characters (80x25).
	// The fourth parameter is the window title.
	// The final parameter says that we'd like the default console to use an 8x16 VGA font. Not so great for games, but easy to read!
	init(config_simple("../assets", 80, 25, "RLTK Hello World", "8x16"));

	entity_t test;
	test.assign(position_component{1,1});
	test.assign(position_component{1,1});
	test.assign(position_component2{1,1});


	// Enter the main loop. "tick" is the function we wrote above.
	run(tick);

    return 0;
}
