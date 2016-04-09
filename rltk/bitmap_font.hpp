#pragma once
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Provides a wrapper for bitmap fonts.
 */

#include <string>
#include <utility>

namespace rltk {

struct bitmap_font {
	bitmap_font() : texture_tag(""), character_size{0,0} {}
	bitmap_font(const std::string tag, const int width, const int height) : texture_tag(tag), character_size({width,height}) {}

	std::string texture_tag;
	std::pair<int,int> character_size;
};

bitmap_font * get_bitmap_font(const std::string font_tag);
void register_bitmap_font(const std::string font_tag, const std::string texture_tag, int width=8, int height=8);
void register_bitmap_font_load(const std::string font_tag, const std::string texture_tag, const std::string filename, int width=8, int height=8);

}