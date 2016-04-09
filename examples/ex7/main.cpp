/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Example 7: Introduction to complex GUIs. This example demonstrates how you can create multiple layers,
 * and use call-backs to resize them as the window adjusts. It also displays a layer on top of another,
 * with alpha transparency (useful for effects).
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"
#include <sstream>
#include <iomanip>

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
using namespace rltk::colors;

// For convenience, we'll define our GUI section handles here. These are just ID numbers.
constexpr int TITLE_LAYER = 0;
constexpr int MAIN_LAYER = 1;
constexpr int LOG_LAYER = 2;
constexpr int OVERLAY_LAYER = 3;

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	term(TITLE_LAYER)->clear(vchar{' ', YELLOW, BLUE});
	term(TITLE_LAYER)->print_center(0, "Big 32x32 Title", YELLOW, BLUE);
	term(MAIN_LAYER)->clear(vchar{'.', GREY, BLACK});
	term(MAIN_LAYER)->box(GREY, BLACK, true);
	term(MAIN_LAYER)->set_char(10, 10, vchar{'@', YELLOW, BLACK});
	term(LOG_LAYER)->clear(vchar{' ', WHITE, DARKEST_GREEN});
	term(LOG_LAYER)->box(DARKEST_GREEN, BLACK);
	term(LOG_LAYER)->print(1,1, "Log Entry", LIGHT_GREEN, DARKEST_GREEN);
	term(LOG_LAYER)->print(1,2, "More text!", LIGHT_GREEN, DARKEST_GREEN);
	term(LOG_LAYER)->print(1,3, "Even more...", LIGHT_GREEN, DARKEST_GREEN);
	term(LOG_LAYER)->print(1,4, "... goes here", LIGHT_GREEN, DARKEST_GREEN);
	term(OVERLAY_LAYER)->clear();
	term(OVERLAY_LAYER)->set_char(11, 10, vchar{17, LIGHT_GREEN, BLACK}); // Draw a left arrow
	term(OVERLAY_LAYER)->print(12, 10, "Translucent Tool-tip", LIGHT_GREEN, BLACK);

	std::stringstream ss;
	ss << std::setiosflags(std::ios::fixed) << std::setprecision(0) << (1000.0/duration_ms) << " FPS";
	term(LOG_LAYER)->print(1,6, ss.str(), WHITE, DARKEST_GREEN);
}

// This is called when the screen resizes, to allow the GUI to redefine itself.
void resize_title(layer_t * l, int w, int h) {
	// Simply set the width to the whole window width
	l->w = w;
	l->h = 32; // Not really necessary - here for clarity
}

// This is called when the screen resizes, to allow the GUI to redefine itself.
void resize_main(layer_t * l, int w, int h) {
	// Simply set the width to the whole window width, and the whole window minus 16 pixels (for the heading)
	l->w = w - 160;
	l->h = h - 32; 
	if (l->w < 0) l->w = 160; // If the width is too small with the log window, display anyway.
}

// This is called when the screen resizes, to allow the GUI to redefine itself.
void resize_log(layer_t * l, int w, int h) {
	// Simply set the width to the whole window width, and the whole window minus 16 pixels (for the heading)
	l->w = w - 160;
	l->h = h - 32;

	// If the log window would take up the whole screen, hide it
	if (l->w < 0) {
		l->console->visible = false;
	} else {
		l->console->visible = true;
	}
	l->x = w - 160;
}

// Your main function
int main()
{
	// This time, we're using a full initialization: width, height, window title, and "false" meaning we don't
	// want an automatically generated root console. This is necessary when you want to use the complex layout
	// functions.
	init(config_advanced("../assets"));

	gui->add_layer(TITLE_LAYER, 0, 0, 1024, 32, "32x32", resize_title);
	gui->add_layer(MAIN_LAYER, 0, 32, 1024-160, 768-32, "8x8", resize_main);
	gui->add_layer(LOG_LAYER, 864, 32, 160, 768-32, "8x16", resize_log);
	gui->add_layer(OVERLAY_LAYER, 0, 32, 1024-160, 768-32, "8x8", resize_main); // We re-use resize_main, we want it over the top
	term(OVERLAY_LAYER)->set_alpha(196); // Make the overlay translucent

	run(tick);

    return 0;
}
