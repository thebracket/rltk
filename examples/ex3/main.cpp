/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Example 3: A wandering @ dude, this time using Bresenham's line functions to
 * plot his path through the world. We play around a bit, rendering the destination
 * and path as well as the simple "world" our little @ lives in.
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"

// We're using a deque to represent our path
#include <deque>

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
using namespace rltk::colors;

// For now, we always want our "dude" to be a yellow @ - so he's constexpr
const vchar dude{'@', YELLOW, BLACK};
// We're also going to render our destination as a pink heart. Aww.
const vchar destination{3, MAGENTA, BLACK};
// We'll also render our planned path ahead as a series of stars
const vchar star{'*', GREEN, BLACK};
// The dude's location in X/Y terms
int dude_x = 10;
int dude_y = 10;

// Where the dude would like to go; we'll start with him being happy where he is.
int destination_x = 10;
int destination_y = 10;

// We'll store the path to the goal as a simple queue of x/y (represented as an std::pair of ints).
// A queue naturally represents the task - each step, in order. A deque has the added property
// of being iteratable - so we are using it.
std::deque<std::pair<int,int>> path;

// A default-defined random number generator. You can specify a seed to get
// the same results each time, but for now we're keeping it simple.
random_number_generator rng;

// We want to keep the game running at a steady pace, rather than however
// fast our super graphics card wants to go! To do this, we set a constant
// duration for a "tick" and only process after that much time has elapsed.
// Most roguelikes are turn-based and won't actually use this, but that's
// for a later example when we get to input.
// Note that this is faster than previous examples; I liked it better that way!
constexpr double tick_duration = 30.0;
double tick_time = 0.0;

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	// Rather than clearing the screen to black, we set it to all white dots.
	console->clear(vchar{'.', GREY, BLACK});

	// Increase the tick time by the frame duration. If it has exceeded
	// the tick duration, then we move the @.
	tick_time += duration_ms;
	if (tick_time > tick_duration) {

		// If we're at our destination, we need a new one!
		if ((destination_x == dude_x and destination_y == dude_y) or path.empty()) {
			// We use the RNG to determine where we want to go
			destination_x = rng.roll_dice(1, console->term_width)-1;
			destination_y = rng.roll_dice(1, console->term_height)-1;
			
			// Now we use "line_func". The prototype for this is:
			// void line_func(int x1, int y1, const int x2, const int y2, std::function<void(int, int)> func);
			// What this means in practice is line_func(from_x, from_y, to_x, to_y, callback function for each step).
			// We'll use a lambda for the callback, to keep it inline and tight.
			line_func(dude_x, dude_y, destination_x, destination_y, [] (int nx, int ny) {
				// Simply add the next step to the path
				path.push_back(std::make_pair(nx,ny));
			});
		} else {
			// We aren't there yet, so we follow our path. We take the first element on the list,
			// and then use pop_back to remove it.
			// std::tie is a handy way to extract two parts of an std::pair (or tuple) in one fell swoop.
			std::tie(dude_x, dude_y) = path.front();
			path.pop_front();
		}


		// Important: we clear the tick count after the update.
		tick_time = 0.0;
	}

	// Clipping: keep the dude on the screen. Why are we doing this here, and not
	// after an update? For now, we aren't handling the concept of a map that is larger
	// than the screen - so if the window resizes, the @ gets clipped to a visible area.
	if (dude_x < 0) dude_x = 0;
	if (dude_x > console->term_width) dude_x = console->term_width;
	if (dude_y < 0) dude_y = 0;
	if (dude_y > console->term_height) dude_x = console->term_height;

	// Render our planned path. We're using auto and a range-for to avoid typing all
	// the iterator stuff
	for (auto step : path) {
		console->set_char(console->at(step.first, step.second), star);
	}

	// Render our destination
	console->set_char(console->at(destination_x, destination_y), destination);
	// Finally, we render the @ symbol. dude_x and dude_y are in terminal coordinates.
	console->set_char(console->at(dude_x, dude_y), dude);
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
	register_bitmap_font("8x8", "../assets/terminal8x8.png", 8, 8);

	// Here are some other font-size options:

	//register_bitmap_font_load("16x16", "terminal16x16", "../assets/terminal16x16.png", 16, 16);
	//register_bitmap_font_load("32x32", "terminal32x32", "../assets/terminal32x32.png", 32, 32);

	// Enter the main loop. "tick" is the function we wrote above, and "8x8" is the font's name
	// we specified as the first parameter to the font loader (the second is an internal "texture_tag")
	run(tick, "8x8");

    return 0;
}
