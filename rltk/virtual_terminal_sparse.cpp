#include "virtual_terminal_sparse.hpp"
#include "texture_resources.hpp"

namespace rltk {

void virtual_terminal_sparse::resize_pixels(const int width, const int height) {
	int w = width/font->character_size.first;
	int h = height/font->character_size.second;
	resize_chars(w,h);
}

void virtual_terminal_sparse::resize_chars(const int width, const int height) {
	dirty = true;
	const int num_chars = width*(height+1);
	buffer.resize(num_chars);
	term_width = width;
	term_height = height;
	backing.create(term_width * font->character_size.first, term_height * font->character_size.second);
}

void virtual_terminal_sparse::render(sf::RenderWindow &window) {
	if (!visible) return;

	if (dirty) {
		if (font == nullptr) {
			throw std::runtime_error("Font not loaded: " + font_tag);
		}
		if (tex == nullptr) {
			tex = get_texture(font->texture_tag);
		}
		const int font_width = font->character_size.first;
		const int font_height = font->character_size.second;
		const float fontW = static_cast<float>(font_width);
		const float fontH = static_cast<float>(font_height);
		const sf::Vector2f origin(fontW/2.0f, fontH/2.0f);

		const int space_x = (219 % 16) * font_width;
		const int space_y = (219 / 16) * font_height;

		// We want to iterate the display vector, and put sprites on the screen for each entry
		for (const xchar &target : buffer) {

			const int texture_x = (target.glyph % 16) * font_width;
			const int texture_y = (target.glyph / 16) * font_height;
			sf::Vector2f position((target.x * fontW) + fontW/2.0f, (target.y * fontH) + fontH/2.0f);

			if (target.background) {
				sf::Color bgsfml = color_to_sfml(target.background.get());
				sf::Sprite sprite;
				sprite.setTexture(*tex);
				sprite.setTextureRect(sf::IntRect(space_x, space_y, font_width, font_height));
				sprite.setColor(bgsfml);
				sprite.setOrigin(origin);
				sprite.setPosition(position);
				sprite.setRotation(target.angle);
				window.draw(sprite);
			}
			sf::Color fgsfml = color_to_sfml(target.foreground);
			sf::Sprite sprite;
			sprite.setTexture(*tex);
			sprite.setTextureRect(sf::IntRect(texture_x, texture_y, font_width, font_height));
			sprite.setColor(fgsfml);
			sprite.setOrigin(origin);
			sprite.setPosition(position);
			sprite.setRotation(target.angle);
			window.draw(sprite);

		}
	}

	// Draw the backing
	backing.display();
	sf::Sprite compositor(backing.getTexture());
	compositor.move(offset_x, offset_y);
	compositor.setColor(sf::Color(tint.r, tint.g, tint.b, alpha));
	window.draw(sf::Sprite(compositor));
	dirty = false;
}

}