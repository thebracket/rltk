#include "font_manager.hpp"
#include "texture_resources.hpp"
#include <unordered_map>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

using namespace boost::filesystem;
using boost::property_tree::ptree;
using boost::property_tree::read_json;

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

void register_font_directory(const std::string path) {
	if (!exists(path)) throw std::runtime_error("Font directory does not exist.");
	if (!is_directory(path)) throw std::runtime_error("Font directory is not a directory.");
	const std::string info_file = path + "/fonts.json";
	if (!exists(info_file)) throw std::runtime_error("No fonts.json file in font directory.");

	ptree font_tree;
	read_json(info_file, font_tree);

	ptree::const_iterator end = font_tree.get_child("fonts").end();
    for (ptree::const_iterator it = font_tree.get_child("fonts").begin(); it != end; ++it) {
    	const std::string font_name = it->first;
    	const std::string font_tree_path = "fonts." + font_name + ".";
    	const std::string font_file = font_tree.get<std::string>(font_tree_path + "file");
    	const int width = font_tree.get<int>(font_tree_path + "width");
    	const int height = font_tree.get<int>(font_tree_path + "height");

    	register_font(font_name, path + "/" + font_file, width, height);
    }
}

}