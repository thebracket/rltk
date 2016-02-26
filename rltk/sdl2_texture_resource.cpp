#include "sdl2_texture_resource.hpp"
#include <stdexcept>

namespace rltk {
namespace internal {

texture_resource::texture_resource(SDL_Renderer * renderer, const std::string filename) {
	SDL_Surface * tmp_surface = IMG_Load(filename.c_str());
	if (tmp_surface == nullptr) throw std::runtime_error("Unable to load image: " + filename);
	image = SDL_CreateTextureFromSurface(renderer, tmp_surface);
	SDL_FreeSurface(tmp_surface);
}

texture_resource::~texture_resource() {
	SDL_DestroyTexture(image);
	image = nullptr;
}

}
}

