#pragma once

#include <SDL2/SDL.h>
#include <string>

namespace rltk {
namespace internal {

class sdl2 {
public:
	sdl2();
	~sdl2();

	void present();
	void clear_to_black();

private:
	std::string window_title = "RLTK Window";
	SDL_Window * window;
	SDL_Renderer * renderer;
};

}
}

