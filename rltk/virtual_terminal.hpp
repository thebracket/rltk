#pragma once
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Provides a virtual console
 */

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "bitmap_font.hpp"
 #include "texture.hpp"

namespace rltk {

struct color_t {
	uint8_t r,g,b;
	inline sf::Color as_sfml_color() const { return sf::Color(r,g,b); }
};

struct vchar {
	int glyph;
	color_t foreground;
	color_t background;
};

struct virtual_terminal {
public:
	virtual_terminal(const std::string fontt, const int x=0, const int y=0) : font_tag(fontt), offset_x(x), offset_y(y) {
		font = get_bitmap_font(fontt);
	}

	void resize_pixels(const int width, const int height);
	void resize_chars(const int width, const int height);
	void clear();
	inline int at(const int x, const int y) { return ( y * term_width) + x; }
	void set_char(const int idx, const vchar &target);
	void print(const int x, const int y, const std::string &s, const color_t &fg, const color_t &bg);

	void render(sf::RenderWindow &window);

private:
	std::string font_tag;
	int term_width;
	int term_height;
	int offset_x;
	int offset_y;
	bitmap_font * font = nullptr;
	sf::Texture * tex = nullptr;
	std::vector<vchar> buffer;

};

}