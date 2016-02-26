#pragma once

#include <string>
#include "sdl2_texture_resource.hpp"

namespace rltk {
namespace internal {

void load_texture_resource(SDL_Renderer * renderer, const std::string tag, const std::string filename);
SDL_Texture * get_texture(const std::string tag);

}
}

