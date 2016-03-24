/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Provides support for complicated GUIs.
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <unordered_map>
#include <memory>
#include "virtual_terminal.hpp"

namespace rltk {

struct layer_t {
	layer_t(const int X, const int Y, const int W, const int H, std::string font_name, std::function<void(layer_t *,int,int)> resize_fun, bool render_background=true) :
		x(X), y(Y), w(W), h(H), font(font_name), resize_func(resize_fun), has_background(render_background) 
	{
		console = std::make_unique<virtual_terminal>(font_name, x, y, has_background);
	    console->resize_pixels(w, h);
	}

	int x;
	int y;
	int w;
	int h;
	std::string font;

	std::function<void(layer_t *,int,int)> resize_func;
	std::unique_ptr<virtual_terminal> console;
	bool has_background;

	void on_resize(const int width, const int height) {
		resize_func(this, width, height);
		if (console->visible) {
			console->set_offset(x,y);
			console->resize_pixels(w, h);
		}
	}

	void render(sf::RenderWindow &window) {
		console->render(window);
	}
};

struct gui_t {
public:
	gui_t(const int w, const int h) : screen_width(w), screen_height(h) {}
	void on_resize(const int w, const int h);
	void render(sf::RenderWindow &window);

	void add_layer(const int handle, const int X, const int Y, const int W, const int H, std::string font_name, std::function<void(layer_t *,int,int)> resize_fun, bool has_background=true, const int order=-1);
	void delete_layer(const int handle);
	layer_t * get_layer(const int handle);

private:
	int screen_width;
	int screen_height;
	int render_order = 0;

	std::unordered_map<int, layer_t> layers;
};

}