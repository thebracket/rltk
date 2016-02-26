#include "rltk.hpp"
#include "config.hpp"
#include "sdl2.hpp"
#include "vterm.hpp"
#include "colors.hpp"
#include <stdexcept>
#include <iostream>

namespace rltk {

namespace detail {
	int screen_width = 0;
	int screen_height = 0;

	rltk::vterm::layer root_console;
}

void init(tick_callback_t tick_handler, const int width, const int height, const std::string title) {
	config::window_title = title;
	config::initial_height = height;
	config::initial_width = width;
	config::main_callback = tick_handler;
}

inline void render_console(rltk::internal::sdl2 &sdl, rltk::vterm::layer &layer) {
	SDL_SetRenderDrawColor(sdl.renderer, 0, 0, 0, 255);
	SDL_Texture * font = rltk::internal::get_texture("8x8");
	if (font == nullptr) throw std::runtime_error("Unable to access font.");
	const int ascii_height = layer.terminal_size.second;
	const int ascii_width = layer.terminal_size.first;

	for (int y=0; y<ascii_height; ++y) {
		for (int x=0; x<ascii_width; ++x) {
			const int screen_x = x * 8;
			const int screen_y = y * 8;
			const char_t target = layer.buffer[layer.idx(x,y)];
			const int texture_x = (target.glyph % 16) * 8;
			const int texture_y = (target.glyph / 16) * 8;
			
			SDL_Rect dst_rec{screen_x, screen_y, 8, 8 };
			SDL_Rect src_rec{texture_x, texture_y, 8, 8};
			//SDL_SetTextureColorMod(font, 255, 255, 255);
			SDL_RenderCopy(sdl.renderer, font, &src_rec, &dst_rec);
		}
	} 
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
			auto screen_size = sdl.get_screen_size();
	                std::tie(detail::screen_width, detail::screen_height) = screen_size;
			detail::root_console.resize(screen_size);
		}

		// Call game logic
		quitting = config::main_callback(duration_ms);
		if (events.request_quit) quitting = true;

		// Rendering
		render_console(sdl, detail::root_console);
		sdl.present();

		// Timer
		duration_ms = ((clock() - start_time) * 1000.0) / CLOCKS_PER_SEC;
	}
}

std::pair<int, int> get_screen_size_px() {
	return std::make_pair(detail::screen_width, detail::screen_height);
}

void print_to_root(int x, int y, std::string t) {
	detail::root_console.print(x, y, t, color::white, color::black);
}

}

