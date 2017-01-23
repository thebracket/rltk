#pragma once
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 */

#include <string>
#include <functional>
#include <SFML/Graphics.hpp>
#include "font_manager.hpp"
#include "texture_resources.hpp"
#include "virtual_terminal.hpp"
#include "colors.hpp"
#include "rng.hpp"
#include "geometry.hpp"
#include "path_finding.hpp"
#include "input_handler.hpp"
#include "visibility.hpp"
#include "gui.hpp"
#include "ecs.hpp"
#include "perlin_noise.hpp"
#include "serialization_utils.hpp"
#include "rexspeeder.hpp"
#include "scaling.hpp"

namespace rltk {

/* 
 * Defines a simple configuration to get most people started. 8x8 font, window size 1024x768. 
 */
struct config_simple_px {
	config_simple_px(const std::string fontpath, const int width=1024, const int height=768, const std::string title="RLTK Roguelike", 
		const std::string font="8x8", bool full_screen=false) :
		font_path(fontpath), width_px(width), height_px(height), window_title(title), root_font(font), fullscreen(full_screen) {}

	const std::string font_path;
	const int width_px;
	const int height_px;
	const std::string window_title;
	const std::string root_font;
	const bool fullscreen;
};

/* 
 * Defines a simple configuration to get most people started. 8x8 font, window size 128x96 (which happens to be 1024x768) 
 */
struct config_simple {
	config_simple(const std::string fontpath, const int width_term=128, const int height_term=96, 
		const std::string title="RLTK Roguelike", const std::string font="8x8", bool full_screen=false) :
		font_path(fontpath), width(width_term), height(height_term), window_title(title), root_font(font), fullscreen(full_screen) {}

	const std::string font_path;
	const int width;
	const int height;
	const std::string window_title;
	const std::string root_font;
	const bool fullscreen;
};

/* 
 * Defines an advanced configuration. No root console, so it is designed for the times you want to build your own GUI.
 */
struct config_advanced {
	config_advanced(const std::string fontpath, const int width=1024, const int height=768, const std::string title="RLTK Roguelike",
		bool full_screen=false) :
		font_path(fontpath), width_px(width), height_px(height), window_title(title), fullscreen(full_screen) {}

	const std::string font_path;
	const int width_px;
	const int height_px;
	const std::string window_title;
	const bool fullscreen;
};

/*
 * Bootstrap the system with a simple configuration, specified in pixels.
 * This variant uses terminal coordinates - so specify 80x40 as size and it will scale with the terminal size.
 */
void init(const config_simple &config);

/*
 * Bootstrap the system with a simple configuration, specified in pixels.
 * This variant uses screen coordinates.
 */
void init(const config_simple_px &config);

/*
 * Bootstrap the system with a simple configuration, specified in pixels.
 * This variant doesn't set up much automatically; it's designed for the times you want to define your own GUI.
 */
void init(const config_advanced &config);

/*
 * The main run loop. Calls on_tick each frame. Window can be initially defined with width/height/title, but these
 * have sane defaults to get you started.
 */
void run(std::function<void(double)> on_tick);

/*
 * For rendering to the console
 */
extern std::unique_ptr<virtual_terminal> console;

/*
 * In case you want to do some SFML stuff yourself, this provides a pointer to the render window.
 */
sf::RenderWindow * get_window();

/*
 * For GUI manipulation
 */
extern std::unique_ptr<gui_t> gui;

/*
 * Convenience function to quickly get a GUI layer
 */
inline layer_t * layer(const int &handle) { return gui->get_layer(handle); }
inline virtual_terminal * term(const int &handle) { return gui->get_layer(handle)->console.get(); }
inline virtual_terminal_sparse * sterm(const int &handle) { return gui->get_layer(handle)->sconsole.get(); }

/* Request a screenshot */
void request_screenshot(const std::string &filename);

/* Lifecycle hooks, for example to integrate ImGui with your application. */
extern std::function<bool(sf::Event)> optional_event_hook;
extern std::function<void()> optional_display_hook;
}
