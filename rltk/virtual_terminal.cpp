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
	backing.create(term_width * font->character_size.first, term_height * font->character_size.second);
}

void virtual_terminal::clear() {
	std::fill(buffer.begin(), buffer.end(), vchar{ 32, {255,255,255}, {0,0,0} });
}

void virtual_terminal::clear(const vchar &target) {
	std::fill(buffer.begin(), buffer.end(), target);
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

	const int space_x = (219 % 16) * font_width;
	const int space_y = (219 / 16) * font_height;

	backing.clear();

	int idx = 0;
	for (int y=0; y<term_height; ++y) {
		const int screen_y = (y * font_height);
		for (int x=0; x<term_width; ++x) {
			const vchar target = buffer[idx];
			const int texture_x = (target.glyph % 16) * font_width;
			const int texture_y = (target.glyph / 16) * font_height;
			const int screen_x = (x * font_width);
			sf::Vector2f pos(screen_x, screen_y);

			// Draw the background
			if (has_background) {
				sf::Sprite bg;
				bg.setTexture(*tex);
				bg.setTextureRect(sf::IntRect(space_x, space_y, font_width, font_height));
				bg.move(pos);
				bg.setColor(color_to_sfml(target.background));
				backing.draw(bg);
			}

			// Draw the foreground
			sf::Sprite sprite;
			sprite.setTexture(*tex);
			sprite.setTextureRect(sf::IntRect(texture_x, texture_y, font_width, font_height));
			sprite.move(pos);
			sprite.setColor(color_to_sfml(target.foreground));
			backing.draw(sprite);

			++idx;
		}
	}

	backing.display();
	sf::Sprite compositor(backing.getTexture());
	compositor.move(offset_x, offset_y);
	compositor.setColor(sf::Color(tint.r, tint.g, tint.b, alpha));
	window.draw(sf::Sprite(compositor));
}

}