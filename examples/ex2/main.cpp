/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Example 2: A wandering @ dude, demonstrating the random number generator,
 * character rendering, directed screen clearing, and frame-rate independent
 * tick lengths.
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"

// We're using a stringstream to build the hello world message.
#include <sstream>

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;

// For now, we always want our "dude" to be a yellow @ - so he's constexpr
constexpr vchar dude{'@', YELLOW, BLACK};
// The dude's location in X/Y terms
int dude_x = 10;
int dude_y = 10;

// A default-defined random number generator. You can specify a seed to get
// the same results each time, but for now we're keeping it simple.
random_number_generator rng;

// We want to keep the game running at a steady pace, rather than however
// fast our super graphics card wants to go! To do this, we set a constant
// duration for a "tick" and only process after that much time has elapsed.
// Most roguelikes are turn-based and won't actually use this, but that's
// for a later example when we get to input.
constexpr double tick_duration = 100.0;
double tick_time = 0.0;

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	// Rather than clearing the screen to black, we set it to all white dots.
	root_console->clear(vchar{'.', WHITE, BLACK});

	// Increase the tick time by the frame duration. If it has exceeded
	// the tick duration, then we move the @.
	tick_time += duration_ms;
	if (tick_time > tick_duration) {

		// Using the RNG's handy roll_dice function, we roll 1d4. Each option
		// represents a possible move in this case. The function is just like D&D's
		// venerable XdY system - you can roll 10d12 with roll_dice(10,12). You
		// aren't limited to dice sizes that exist or make sense.
		int direction = rng.roll_dice(1,4);
		switch (direction) {
			case 1 : --dude_x; break;
			case 2 : ++dude_x; break;
			case 3 : --dude_y; break;
			case 4 : ++dude_y; break;
		}

		// Important: we clear the tick count after the update.
		tick_time = 0.0;
	}

	// Clipping: keep the dude on the screen. Why are we doing this here, and not
	// after an update? For now, we aren't handling the concept of a map that is larger
	// than the screen - so if the window resizes, the @ gets clipped to a visible area.
	if (dude_x < 0) dude_x = 0;
	if (dude_x > root_console->term_width) dude_x = root_console->term_width;
	if (dude_y < 0) dude_y = 0;
	if (dude_y > root_console->term_height) dude_x = root_console->term_height;

	// Finally, we render the @ symbol. dude_x and dude_y are in terminal coordinates.
	root_console->set_char(root_console->at(dude_x, dude_y), dude);
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
