#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <utility>
#include "sdl2_resource_manager.hpp"

namespace rltk {
namespace internal {

struct events_t {
	bool request_quit = false;
	bool resized = false;

	void reset() {
		request_quit = false;
		resized = false;
	}
};

class sdl2 {
public:
	sdl2();
	~sdl2();

	void poll(events_t &result);

	void present();
	void clear_to_black();
	std::pair<int,int> get_screen_size();

	void load_texture(const std::string tag, const std::string filename);

	std::string window_title = "RLTK Window";
	SDL_Window * window;
	SDL_Renderer * renderer;
};

}
}

