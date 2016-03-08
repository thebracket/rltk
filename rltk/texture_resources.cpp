#include <unordered_map>
#include <stdexcept>
#include "texture_resources.hpp"
#include "texture.hpp"

namespace rltk {

namespace texture_detail {

std::unordered_map<std::string,rltk::texture> atlas;

}

void register_texture(const std::string &filename, const std::string &tag) {
	auto finder = texture_detail::atlas.find(tag);
	if (finder != texture_detail::atlas.end()) {
		throw std::runtime_error("Duplicate resource tag: " + tag);
	}

	texture_detail::atlas[tag] = rltk::texture(filename);
}

sf::Texture * get_texture(const std::string &tag) {
	auto finder = texture_detail::atlas.find(tag);
	if (finder == texture_detail::atlas.end()) {
		throw std::runtime_error("Unable to find resource tag: " + tag);
	} else {
		return finder->second.tex.get();
	}
}

}
