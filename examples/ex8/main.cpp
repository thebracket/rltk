/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Example 7: Advanced GUI with retained-mode GUI elements and an owner-draw background.
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"
#include <sstream>
#include <iomanip>

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
using namespace rltk::colors;

constexpr int BACKDROP_LAYER = 1;
constexpr int LOG_LAYER = 2;

void resize_bg(layer_t * l, int w, int h) {
	// Use the whole window
	l->w = w;
	l->h = h;
}

void draw_bg(layer_t * l, sf::RenderTexture &window) {
	sf::Texture * bg = get_texture("backdrop");
	sf::Sprite backdrop(*bg);
	window.draw(backdrop);
}

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

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	term(LOG_LAYER)->clear(vchar{' ', WHITE, DARKEST_GREEN});
	term(LOG_LAYER)->box(DARKEST_GREEN, BLACK);
	term(LOG_LAYER)->print(1,1, "Log Entry", LIGHT_GREEN, DARKEST_GREEN);
	term(LOG_LAYER)->print(1,2, "More text!", LIGHT_GREEN, DARKEST_GREEN);
	term(LOG_LAYER)->print(1,3, "Even more...", LIGHT_GREEN, DARKEST_GREEN);
	term(LOG_LAYER)->print(1,4, "... goes here", LIGHT_GREEN, DARKEST_GREEN);

	std::stringstream ss;
	ss << std::setiosflags(std::ios::fixed) << std::setprecision(0) << (1000.0/duration_ms) << " FPS";
	term(LOG_LAYER)->print(1,6, ss.str(), WHITE, DARKEST_GREEN);
}

// Your main function
int main()
{
	// This time, we're using a full initialization: width, height, window title, and "false" meaning we don't
	// want an automatically generated root console. This is necessary when you want to use the complex layout
	// functions.
	init(config_advanced("../assets"));

	// We're going to be using a bitmap, so we need to load it. The library makes this easy:
	register_texture("../assets/background_image.png", "backdrop");

	// Now we add an owner-draw background layer. "Owner-draw" means that it the library will ask it to
	// draw itself with a call-back function.
	gui->add_owner_layer(BACKDROP_LAYER, 0, 0, 1024, 768, resize_bg, draw_bg);
	gui->add_layer(LOG_LAYER, 864, 32, 160, 768-32, "8x16", resize_log);
	term(LOG_LAYER)->set_alpha(196); // Make the overlay translucent

	// Main loop - calls the 'tick' function you defined for each frame.
	run(tick);

    return 0;
}
