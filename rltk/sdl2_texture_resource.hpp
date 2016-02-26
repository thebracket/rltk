#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

namespace rltk {
namespace internal {

class texture_resource {
public:
	texture_resource() {}
	texture_resource(SDL_Renderer * renderer, const std::string filename);
	~texture_resource();
	SDL_Texture * image = nullptr;
};

}
}
