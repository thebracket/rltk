#include "rltk.hpp"
#include "config.hpp"
#include "sdl2.hpp"

namespace rltk {

void init(std::function<bool()> tick_handler, const int width, const int height, const std::string title) {
	config::window_title = title;
	config::initial_height = height;
	config::initial_width = width;
	config::main_callback = tick_handler;
}

void run() {
	bool quitting = false;

	rltk::internal::sdl2 sdl;

	while (!quitting) {
		quitting = config::main_callback();
		sdl.clear_to_black();
		sdl.present();
	}
}

}

