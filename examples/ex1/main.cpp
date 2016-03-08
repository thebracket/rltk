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

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	// In this case, we just want to print "Hello World" in white on black.
	std::stringstream ss;
	ss << "Frame duration: " << duration_ms << " ms (" << (1000.0/duration_ms) << " FPS).";
	get_root_console()->print(1,1,"Hello World", WHITE, BLACK);
	get_root_console()->print(1,2,ss.str(), YELLOW, BLACK);
}

// Your main function
int main()
{
	// Really rltk::init; initializes the system. You can specify window size and title here
	// as init(1024,768,"My Amazing Game")
	init();

	// Register the font we want to use for ASCII rendering. The extended _load form of the
	// function also loads the bitmap for you. Don't worry about cleaning up - the library
	// does that for you.
	register_bitmap_font_load("8x8", "terminal8x8", "../assets/terminal8x8.png", 8, 8);

	// Here are some other font-size options:

	//register_bitmap_font_load("16x16", "terminal16x16", "../assets/terminal16x16.png", 16, 16);
	//register_bitmap_font_load("32x32", "terminal32x32", "../assets/terminal32x32.png", 32, 32);

	// Enter the main loop. "tick" is the function we wrote above, and "8x8" is the font's name
	// we specified as the first parameter to the font loader (the second is an internal "texture_tag")
	run(tick, "8x8");

    return 0;
}
