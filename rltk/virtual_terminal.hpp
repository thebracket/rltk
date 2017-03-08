#pragma once
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Provides a virtual console
 */

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "font_manager.hpp"
#include "texture.hpp"
#include "color_t.hpp"
#include "colors.hpp"
#include "rexspeeder.hpp"
#include "vchar.hpp"

namespace rltk {

/*
 * Combines the functions/data required to render a virtual terminal, either as root or
 * as a layer. This is generally initialized by the library, not the user.
 */
struct virtual_terminal {
public:
	/*
	 * Constructor to create a virtual terminal. Parameters:
	 *   fontt the tag of the bitmap font to use for the terminal.
	 *   x (defaults to 0); an offset at which to render the terminal.
	 *   y (defaults to 0); an offset at which to render the terminal.
	 *   background; if true, then solid backgrounds will be rendered. If false, then no background rendered.
	 */
	virtual_terminal(const std::string fontt, const int x=0, const int y=0, const bool background=true) : font_tag(fontt), offset_x(x), offset_y(y), has_background(background) {
		font = get_font(fontt);
	}

	/*
	 * Resize the terminal to match width x height pixels.
	 */
	void resize_pixels(const int width, const int height) noexcept;

	/*
	 * Resize the terminal to match width x height virtual characters.
	 */
	void resize_chars(const int width, const int height) noexcept;

	/*
	 * Clears the virtual terminal to black spaces.
	 */
	void clear() noexcept;

	/*
	 * Clears the virtual terminal to a user-provided character.
	 * vchar; the character to which the terminal should be set.
	 */
	void clear(const vchar &target) noexcept;

	/*
	 * Helper function that returns the index of a character location in the backing vector.
	 */
	inline int at(const int x, const int y) noexcept { return ( y * term_width) + x; }

	/*
	 * Set a character at backing-vector idx (use "at" to calculate) to the specified target virtual
	 * character.
	 */
	void set_char(const int idx, const vchar &target) noexcept;

	/*
	 * Set a character at x/y to the specified target virtual character.
	 */
	inline void set_char(const int x, const int y, vchar target) noexcept { set_char(at(x,y), target); }

	/*
	 * Print a string to the terminal, at location x/y, string s, with foreground and background of fg and bg.
	 * If you don't specify colors, it will do white on black.
	 */
	void print(const int x, const int y, const std::string &s, const color_t &fg = colors::WHITE, const color_t &bg = colors::BLACK) noexcept;

	/*
	 * Center a string (relative to the terminal size), and print it at location y.
	 */
	void print_center(const int y, const std::string &s, const color_t &fg = colors::WHITE, const color_t &bg = colors::BLACK) noexcept;

	/*
	 * Draw a box taht encompasses the whole terminal boundary, in color fg/bg. If double_lines is set to true, then
	 * it will use the two-line/thick box characters.
	 */
	inline void box(const color_t &fg = colors::WHITE, const color_t &bg = colors::BLACK, bool double_lines=false) noexcept {
		box (0, 0, term_width-1, term_height-1, fg, bg, double_lines);
	}

	/*
	 * Draw a box at x/y of size w/h, in color fg/bg (or black/white if not specified). If double_lines is true,
	 * it will use the two-line/thick box characters.
	 */
	void box(const int x, const int y, const int w, const int h, const color_t &fg = colors::WHITE, const color_t &bg = colors::BLACK, bool double_lines=false) noexcept;

	/*
	 * Fill a region with a specified character
	 */
	void fill(const int left_x, const int top_y, const int right_x, const int bottom_y, const uint8_t glyph, const color_t &fg = colors::WHITE, const color_t &bg = colors::BLACK) noexcept;

	/*
	 * Draw a REX sprite at the specified location.
	 */
	void draw_sprite(const int x, const int y, xp::rex_sprite &sprite);

	/*
	 * Renders the terminal to the specified renderable. Don't call this directly - the toolkit will take care of it.
	 */
	void render(sf::RenderWindow &window);

	/*
	 * Sets the global translucency level for the console. You can use this to make a translucent console layer on top of
	 * other items.
	 */
	inline void set_alpha(const uint8_t new_alpha) noexcept { alpha = new_alpha; }

	/*
	 * Use this to tint the entire rendering of the console.
	 */
	inline void set_tint(const color_t new_tint) noexcept { tint = new_tint; }

	/*
	 * Use this to move the terminal in x/y screen pixels.
	 */
	inline void set_offset(int x, int y) noexcept { offset_x = x; offset_y = y; };

	/*
	 * This gets the current font size in pixels, first is width, second is height.
	 */
	inline std::pair<int,int> get_font_size() noexcept { return font->character_size; }

	inline std::string get_font_tag() noexcept { return font->texture_tag; }

	int term_width;
	int term_height;
	bool visible = true;
	bool dirty = true; // Flag for requiring a re-draw

private:
	sf::RenderTexture backing;
	sf::VertexArray vertices;
	std::string font_tag;
	int offset_x;
	int offset_y;
	uint8_t alpha = 255;
	color_t tint{255,255,255};
	bool has_background;
	bitmap_font * font = nullptr;
	sf::Texture * tex = nullptr;
	std::vector<vchar> buffer;

};

}