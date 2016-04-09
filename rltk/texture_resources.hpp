#pragma once
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Provides a global texture atlas.
 */

#include <string>
#include <SFML/Graphics.hpp>

namespace rltk {

void register_texture(const std::string &filename, const std::string &tag);
sf::Texture * get_texture(const std::string &tag);

}
