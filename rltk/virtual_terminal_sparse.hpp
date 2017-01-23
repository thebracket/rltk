/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Provides a virtual console, in 'sparse' mode: it doesn't assume that you have a regular grid,
 * supports off-alignment rendering, rotation and similar. It is optimized for use with a few
 * characters, rather than a complete grid. A common usage would be as a layer for PCs/NPCs,
 * rendered over a regular console grid.
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "font_manager.hpp"
#include "texture.hpp"
#include "color_t.hpp"
#include "colors.hpp"

namespace rltk {

/* 
 * Represents a character on a sparse virtual terminal. 
 */
struct xchar {
	xchar() {}
	xchar(const int Glyph, const color_t fg, const float X, const float Y) : glyph(Glyph), foreground(fg), x(X), y(Y) {}
	xchar(const int Glyph, const color_t fg, const float X, const float Y, const int ANGLE) : glyph(Glyph), foreground(fg), x(X), y(Y), angle(ANGLE) {}
	int glyph; // Glyph to render
	color_t foreground; // Foreground color
	float x = 0.0f; // Provided as floats to allow for partial character movement/sliding
	float y = 0.0f;
	int angle = 0; // Rotation angle in degrees, defaults to 0 - not rotated
	unsigned char opacity = 255;
	bool has_background = false;
	color_t background; // If provided, a background is drawn
};

struct virtual_terminal_sparse {

	virtual_terminal_sparse(const std::string fontt, const int x=0, const int y=0) : font_tag(fontt), offset_x(x), offset_y(y) {
		font = get_font(fontt);
	}

	void resize_pixels(const int width, const int height);
	void resize_chars(const int width, const int height);
	inline void clear() {
		dirty = true;
		buffer.clear();
	}
	inline void add(const xchar target) {
		dirty = true;
		buffer.push_back(target);
	}
	void render(sf::RenderWindow &window);

	int term_width;
	int term_height;
	bool visible = true;
	bool dirty = true; // Flag for requiring a re-draw

private:
	std::string font_tag;
	int offset_x;
	int offset_y;
	uint8_t alpha = 255;
	color_t tint{255,255,255};
	bitmap_font * font = nullptr;
	sf::Texture * tex = nullptr;
	std::vector<xchar> buffer;
	sf::RenderTexture backing;
};

}
