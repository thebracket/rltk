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

random_number_generator rng;

constexpr int BACKDROP_LAYER = 1;
constexpr int LOG_LAYER = 2;
constexpr int RETAINED_TEST_LAYER = 3;

constexpr int TEST_BOUNDARY_BOX = 1;
constexpr int TEST_STATIC_TEST = 2;
constexpr int TEST_MOUSE_HOVER = 3;
constexpr int TEST_CHECKBOX = 4;
constexpr int TEST_RADIOSET = 5;
constexpr int TEST_HBAR = 6;
constexpr int TEST_VBAR = 7;

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

void resize_retained(layer_t * l, int w, int h) {
	// Do nothing - we'll just keep on rendering away.
	l->x = 100;
	l->y = 100;
	l->w = 400;
	l->h = 200;
}

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
	term(LOG_LAYER)->clear(vchar{' ', WHITE, DARKEST_GREEN});
	term(LOG_LAYER)->box(DARKEST_GREEN, BLACK);

	layer_t * retained = layer(RETAINED_TEST_LAYER);

	if ( retained->control<gui_checkbox_t>(TEST_CHECKBOX)->checked ) {
		term(LOG_LAYER)->print(1,1, "Checked", LIGHTEST_GREEN, DARKEST_GREEN);
	} else {
		term(LOG_LAYER)->print(1,1, "Not Checked", DARK_GREEN, DARKEST_GREEN);
	}

	std::stringstream radio_ss;
	radio_ss << "Option: " << retained->control<gui_radiobuttons_t>(TEST_RADIOSET)->selected_value;
	term(LOG_LAYER)->print(1,2, radio_ss.str(), LIGHT_GREEN, DARKEST_GREEN);
	term(LOG_LAYER)->print(1,3, "Even more...", LIGHT_GREEN, DARKEST_GREEN);
	term(LOG_LAYER)->print(1,4, "... goes here", LIGHT_GREEN, DARKEST_GREEN);

	std::stringstream ss;
	ss << std::setiosflags(std::ios::fixed) << std::setprecision(0) << (1000.0/duration_ms) << " FPS";
	term(LOG_LAYER)->print(1,6, ss.str(), WHITE, DARKEST_GREEN);

	if (rng.roll_dice(1,20)==1) {
		retained->control<gui_hbar_t>(TEST_HBAR)->value = rng.roll_dice(1,100);
		retained->control<gui_vbar_t>(TEST_VBAR)->value = rng.roll_dice(1,100);
	}
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
	gui->add_layer(RETAINED_TEST_LAYER, 100, 100, 400, 200, "8x16", resize_retained);

	// To reduce typing, grab a pointer to the retained layer:
	layer_t * retained = layer(RETAINED_TEST_LAYER);

	// Now we build some retained-mode controls. These don't require additional work during rendering
	// Note that we are providing a handle to the control. That lets us access it later with 
	// layer(layerhandle)->control(controlhandle). It's up to you to store the handles; they can be any
	// int.
	retained->add_boundary_box(TEST_BOUNDARY_BOX, true, DARK_GREY, BLACK);
	retained->add_static_text(TEST_STATIC_TEST, 1, 1, "Retained Mode Static Text", YELLOW, BLACK);

	// For this control, we'll define an on-mouse-over. We're using a lambda, but it could be any function
	// with that takes a gui_control_t * as a parameter. We'll also use "on render start" to define a function
	// run when the control rendering starts.
	retained->add_static_text(TEST_MOUSE_HOVER, 1, 2, "Hover the mouse over me!", WHITE, BLACK);
	retained->control(TEST_MOUSE_HOVER)->on_render_start = [] (gui_control_t * control) {
		auto static_text = static_cast<gui_static_text_t *>(control);
		static_text->background = BLACK;
		static_text->text = "Hover the mouse over me!";
	};
	retained->control(TEST_MOUSE_HOVER)->on_mouse_over = [] (gui_control_t * control, int terminal_x, int terminal_y) {
		auto static_text = static_cast<gui_static_text_t *>(control);
		static_text->background = RED;
		static_text->text = "Why Hello There!        ";
	};

	// A checkbox
	retained->add_checkbox(TEST_CHECKBOX, 1, 3, "I'm a checkbox - click me!", false, LIGHT_GREEN, BLACK);

	// A radioset
	retained->add_radioset(TEST_RADIOSET, 1, 5, "Test radio buttons", CYAN, BLACK, {
		{true, "Option A", 0}, {false, "Option B", 1}, {false, "Option C", 2}
	});

	// Add a horizontal and vertical color bar (e.g. health)
	retained->add_hbar(TEST_HBAR, 1, 9, 48, 0, 100, 50, color_t(128,0,0), color_t(255,0,0), color_t(128,128,128), color_t(64,64,64), WHITE, "Health: ");
	retained->add_vbar(TEST_VBAR, 48, 1, 7, 0, 100, 50, color_t(0,0,128), color_t(0,0,128), color_t(128,128,128), color_t(64,64,64), CYAN, "");

	// Main loop - calls the 'tick' function you defined for each frame.
	run(tick);

    return 0;
}
