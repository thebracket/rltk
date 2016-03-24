/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Example 1: A truly minimal hello world root console, demonstrating
 * how to get started with RLTK.
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
	std::stringstream ss;
	ss << "Frame duration: " << duration_ms << " ms (" << (1000.0/duration_ms) << " FPS).";
	console->print(1,1,"Hello World", WHITE, BLACK);
	console->print(1,2,ss.str(), YELLOW, BLUE);
}

// Your main function
int main()
{
	// Really rltk::init; initializes the system. You can specify window size and title here
	// as init(1024,768,"My Amazing Game")
	init("../assets");

	// Enter the main loop. "tick" is the function we wrote above, and "8x8" is the font's name
	// we specified as the first parameter to the font loader (the second is an internal "texture_tag")
	run(tick);

    return 0;
}
