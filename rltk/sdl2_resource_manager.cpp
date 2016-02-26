#include "sdl2_resource_manager.hpp"
#include <unordered_map>
#include <memory>

namespace rltk {
namespace internal {

std::unordered_map<std::string, std::unique_ptr<texture_resource>> textures;

void load_texture_resource(SDL_Renderer * renderer, const std::string tag, const std::string filename) {
	textures[tag] = std::make_unique<texture_resource>(renderer, filename);
}

SDL_Texture * get_texture(const std::string tag) {
	auto finder = textures.find(tag);
	if (finder != textures.end()) {
		 return finder->second->image;
	} else {
		return nullptr;
	}
}

}
}

