/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Font manager
 */

#pragma once

#include <string>
#include <utility>

namespace rltk {

struct bitmap_font {
	bitmap_font(const std::string tag, const int width, const int height) : texture_tag(tag), character_size({width,height}) {}

	const std::string texture_tag;
	const std::pair<int,int> character_size;
};

void register_font_directory(const std::string path);
bitmap_font * get_font(const std::string font_tag);
void register_font(const std::string font_tag, const std::string filename, int width=8, int height=8);


}