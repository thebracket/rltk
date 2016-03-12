#pragma once
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 */

#include <string>
#include <functional>
#include <SFML/Graphics.hpp>
#include "texture_resources.hpp"
#include "virtual_terminal.hpp"
#include "colors.hpp"
#include "rng.hpp"
#include "geometry.hpp"
#include "path_finding.hpp"
#include "input_handler.hpp"
#include "visibility.hpp"

namespace rltk {

/*
 * Bootstrap the system with a window size and title (all optional params; leave them out and get 1024x768)
 */
void init(const int window_width=1024, const int window_height=768, const std::string window_title="RLTK Roguelike", const bool use_root_console=true);

/*
 * The main run loop. Calls on_tick each frame. Window can be initially defined with width/height/title, but these
 * have sane defaults to get you started.
 */
void run(std::function<void(double)> on_tick, const std::string root_console_font);

/*
 * For rendering to the console
 */
extern std::unique_ptr<virtual_terminal> console;

/*
 * In case you want to do some SFML stuff yourself, this provides a pointer to the render window.
 */
sf::RenderWindow * get_window();

}
