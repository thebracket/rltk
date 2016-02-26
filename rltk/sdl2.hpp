#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <utility>

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

private:
	std::string window_title = "RLTK Window";
	SDL_Window * window;
	SDL_Renderer * renderer;
};

}
}

