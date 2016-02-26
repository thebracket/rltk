#pragma once

#include <unordered_map>
#include <string>
#include "sdl2_texture_resource.hpp"

namespace rltk {
namespace internal {

extern std::unordered_map<std::string, texture_resource> textures;

void load_texture(SDL_Renderer * renderer, const std::string tag, const std::string filename);
SDL_Texture * get_texture(const std::string tag);

}
}

