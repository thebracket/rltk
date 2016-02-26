#include "sdl2_resource_manager.hpp"

namespace rltk {
namespace internal {

std::unordered_map<std::string, texture_resource> textures;

void load_texture(SDL_Renderer * renderer, const std::string tag, const std::string filename) {
	textures[tag] = texture_resource(renderer, filename);
}

SDL_Texture * get_texture(const std::string tag) {
	auto finder = textures.find(tag);
	if (finder != textures.end()) return finder->second.image;
	return nullptr;
}

}
}

