#include "rltk.hpp"
#include "config.hpp"
#include "sdl2.hpp"

namespace rltk {

namespace detail {
	int screen_width = 0;
	int screen_height = 0;
}

void init(tick_callback_t tick_handler, const int width, const int height, const std::string title) {
	config::window_title = title;
	config::initial_height = height;
	config::initial_width = width;
	config::main_callback = tick_handler;
}

void run() {
	bool quitting = false;

	rltk::internal::sdl2 sdl;
	rltk::internal::events_t events;

	// Load fonts
	sdl.load_texture("8x8", "terminal8x8.png");
	sdl.load_texture("16x16", "terminal16x16.png");
	sdl.load_texture("32x32", "terminal32x32.png");

	double duration_ms = 0.0;
	while (!quitting) {
		// Timers
		clock_t start_time = clock();

		// Clear screen
		sdl.clear_to_black();

		// Inputs
		events.reset();
		sdl.poll(events);

		// Update Informational storage
		if (detail::screen_width == 0 or events.resized) {
	                std::tie(detail::screen_width, detail::screen_height) = sdl.get_screen_size();
		}

		// Call game logic
		quitting = config::main_callback(duration_ms);
		if (events.request_quit) quitting = true;

		// Rendering
		sdl.present();

		// Timer
		duration_ms = ((clock() - start_time) * 1000.0) / CLOCKS_PER_SEC;
	}
}

std::pair<int, int> get_screen_size_px() {
	return std::make_pair(detail::screen_width, detail::screen_height);
}

}

