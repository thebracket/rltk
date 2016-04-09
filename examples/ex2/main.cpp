/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Example 2: A wandering @ dude, demonstrating the random number generator,
 * character rendering, directed screen clearing, and frame-rate independent
 * tick lengths.
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
using namespace rltk::colors;

// For now, we always want our "dude" to be a yellow @ - so he's constexpr
const vchar dude{'@', YELLOW, BLACK};
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
constexpr double tick_duration = 5.0;
double tick_time = 0.0;

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	// Rather than clearing the screen to black, we set it to all white dots. We only want
	// to do this if something has forced the screen to re-render (such as re-sizing)
	if (console->dirty) console->clear(vchar{'.', GREY, BLACK});

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

		// Clear the console, which has the nice side effect of setting the terminal
		// to dirty.
		console->clear(vchar{'.', GREY, BLACK});
		// Clipping: keep the dude on the screen. Why are we doing this here, and not
		// after an update? For now, we aren't handling the concept of a map that is larger
		// than the screen - so if the window resizes, the @ gets clipped to a visible area.
		if (dude_x < 0) dude_x = 0;
		if (dude_x > console->term_width) dude_x = console->term_width;
		if (dude_y < 0) dude_y = 0;
		if (dude_y > console->term_height) dude_x = console->term_height;

		// Finally, we render the @ symbol. dude_x and dude_y are in terminal coordinates.
		console->set_char(console->at(dude_x, dude_y), dude);
	}
}

// Your main function
int main()
{
	// Initialize with defaults.
	init(config_simple_px("../assets"));

	// Enter the main loop. "tick" is the function we wrote above.
	run(tick);

    return 0;
}
