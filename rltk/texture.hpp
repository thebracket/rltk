/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Provides RIAA wrapper for a texture.
 */

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <stdexcept>
#include <boost/filesystem.hpp>

namespace rltk {

struct texture {
	texture() {}
	texture(const std::string &filename) {
		if (!boost::filesystem::exists(filename)) {
			throw std::runtime_error("Load texture error, file not found: " + filename);
		}

		tex = std::make_unique<sf::Texture>();
		if (!tex->loadFromFile(filename))
		{
		    throw std::runtime_error("Unable to load texture from: " + filename);
		}
	}
	std::unique_ptr<sf::Texture> tex;
};

}
