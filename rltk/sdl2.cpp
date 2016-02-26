#include "sdl2.hpp"
#include <stdexcept>
#include "sdl2_resource_manager.hpp"

namespace rltk {
namespace internal {

sdl2::sdl2() {
	window = SDL_CreateWindow(window_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if (window == nullptr) throw std::runtime_error("Unable to create window.");

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) throw std::runtime_error("Unable to create renderer.");
}

sdl2::~sdl2() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void sdl2::present() {
	SDL_RenderPresent(renderer);
}

void sdl2::clear_to_black() {
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderClear(renderer);
}

std::pair<int,int> sdl2::get_screen_size() {
        int w,h;
        SDL_GetWindowSize(window, &w, &h);
        return std::make_pair(w,h);
}

void sdl2::poll(events_t &result) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			result.request_quit = true;
		}
		if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_RESIZED or event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				result.resized = true;
			}
		}
	}
}

void sdl2::load_texture(const std::string tag, const std::string filename) {
	load_texture_resource(renderer, tag, filename);
}

}
}

