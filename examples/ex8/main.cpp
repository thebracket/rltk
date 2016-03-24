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

void resize_bg(layer_t * l, int w, int h) {
	// Use the whole window
	l->w = w;
	l->h = h;
}

void draw_bg(layer_t * l, sf::RenderWindow &window) {
	sf::Texture * bg = get_texture("backdrop");
	sf::Sprite backdrop(*bg);
	window.draw(backdrop);
}

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
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

	// Main loop - calls the 'tick' function you defined for each frame.
	run(tick);

    return 0;
}
