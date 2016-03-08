#include "virtual_terminal.hpp"
#include "texture_resources.hpp"
#include <algorithm>
#include <stdexcept>

namespace rltk {

void virtual_terminal::resize_pixels(const int width, const int height) {
	int w = width/font->character_size.first;
	int h = height/font->character_size.second;
	resize_chars(w,h);
}

void virtual_terminal::resize_chars(const int width, const int height) {
	const int num_chars = width*height;
	buffer.resize(num_chars);
	term_width = width;
	term_height = height;
}

void virtual_terminal::clear() {
	std::fill(buffer.begin(), buffer.end(), vchar{ 32, {255,255,255}, {0,0,0} });
}

void virtual_terminal::set_char(const int idx, const vchar &target) {
	buffer[idx] = target;
}

void virtual_terminal::print(const int x, const int y, const std::string &s, const color_t &fg, const color_t &bg) {
	int idx = at(x,y);
	for (int i=0; i<s.size(); ++i) {
		buffer[idx] = { s[i], fg, bg };
		++idx;
	}
}

void virtual_terminal::render(sf::RenderWindow &window) {
	if (font == nullptr) {
		throw std::runtime_error("Font not loaded: " + font_tag);
	}
	if (tex == nullptr) {
		tex = get_texture(font->texture_tag);
	}
	const int font_width = font->character_size.first;
	const int font_height = font->character_size.second;
	const int font_width_2 = font_width * 2;
	const int font_height_2 = font_height * 2;

	int idx = 0;
	for (int y=0; y<term_height; ++y) {
		for (int x=0; x<term_width; ++x) {
			const vchar target = buffer[idx];
			const int texture_x = (target.glyph % 16) * font_width;
			const int texture_y = (target.glyph / 16) * font_height;

			sf::Sprite sprite;
			sprite.setTexture(*tex);
			sprite.setTextureRect(sf::IntRect(texture_x, texture_y, font_width, font_height));
			sprite.move(sf::Vector2f(x*font_width, y*font_height));
			window.draw(sprite);

			++idx;
		}
	}
}

}