#include "rltk.hpp"
#include "config.hpp"
#include "sdl2.hpp"

namespace rltk {

void init(tick_callback_t tick_handler, const int width, const int height, const std::string title) {
	config::window_title = title;
	config::initial_height = height;
	config::initial_width = width;
	config::main_callback = tick_handler;
}

void run() {
	bool quitting = false;

	rltk::internal::sdl2 sdl;

	double duration_ms = 0.0;
	while (!quitting) {
		clock_t start_time = clock();

		// Clear screen
		sdl.clear_to_black();

		// Inputs

		// Call game logic
		quitting = config::main_callback(duration_ms);

		// Rendering
		sdl.present();

		// Timer
		duration_ms = ((clock() - start_time) * 1000.0) / CLOCKS_PER_SEC;
	}
}

}

