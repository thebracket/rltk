#include "font_manager.hpp"
#include "texture_resources.hpp"
#include <unordered_map>
#include <stdexcept>

namespace rltk {

namespace font_detail {
std::unordered_map<std::string, rltk::bitmap_font> atlas;
}

bitmap_font * get_font(const std::string font_tag) {
	auto finder = font_detail::atlas.find(font_tag);
	if (finder == font_detail::atlas.end()) {
		throw std::runtime_error("Unable to locate bitmap font with tag " + font_tag);
	} else {
		return &finder->second;
	}
}

inline void check_for_duplicate_font(const std::string &tag) {
	auto finder = font_detail::atlas.find(tag);
	if (finder != font_detail::atlas.end()) {
		throw std::runtime_error("Attempting to insert duplicate font with tag " + tag);
	}
}

inline void check_texture_exists(const std::string &texture_tag) {
	if (get_texture(texture_tag) == nullptr) {
		throw std::runtime_error("No such texture resource: " + texture_tag);
	}
}

void register_font(const std::string font_tag, const std::string filename, int width, int height) {
	const std::string texture_tag = "font_tex_" + filename;
	check_for_duplicate_font(font_tag);
	register_texture(filename, texture_tag);
	check_texture_exists(texture_tag);	
	font_detail::atlas.emplace(std::make_pair(font_tag, bitmap_font(texture_tag, width, height)));
}


}