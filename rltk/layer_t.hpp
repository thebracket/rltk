/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Layer type used by the GUI
 */
#pragma once

#include "virtual_terminal.hpp"
#include "gui_control_t.hpp"

namespace rltk {

/*
 * A renderable layer. You won't use this type directly.
 */
struct layer_t {
	/* This specialization is for generic consoles */
	layer_t(const int X, const int Y, const int W, const int H, std::string font_name, std::function<void(layer_t *,int,int)> resize_fun, bool render_background=true) :
		x(X), y(Y), w(W), h(H), font(font_name), resize_func(resize_fun), has_background(render_background) 
	{
		console = std::make_unique<virtual_terminal>(font_name, x, y, has_background);
	    console->resize_pixels(w, h);
	}

	/* This specialization is for owner-draw panels */
	layer_t(const int X, const int Y, const int W, const int H, std::function<void(layer_t *,int,int)> resize_fun, std::function<void(layer_t *, sf::RenderTexture &)> owner_draw_fun) :
		x(X), y(Y), w(W), h(H), resize_func(resize_fun), owner_draw_func(owner_draw_fun)
	{
	}

	// The bounding box of the layer
	int x;
	int y;
	int w;
	int h;

	// Font tag - used only if there is a console
	std::string font;

	// Callbacks:
	// resize_func is called when the window changes size. It receives the WINDOW dimensions - it's up to you to
	// determine how to lay things out.
	std::function<void(layer_t *,int,int)> resize_func;

	// Passed through to virtual console; if false then no background will be rendered (helpful for text overlays)
	bool has_background;

	// owner_draw_func is used only for owner draw layers, and is called at render time.
	std::function<void(layer_t *, sf::RenderTexture &)> owner_draw_func;

	// If a console is present, this is it.
	std::unique_ptr<virtual_terminal> console;

	// If retained-mode controls are present, they are in here.
	std::vector<gui_control_t> controls;

	// Used for owner-draw layers. We need to render to texture and then compose to:
	// a) permit threading, should you so wish (so there is a single composite run)
	// b) allow the future "effects" engine to run.
	std::unique_ptr<sf::RenderTexture> backing;

	// Used by the owner-draw code to ensure that a texture is available for use
	void make_owner_draw_backing();

	// Called by GUI when a resize event occurs.
	void on_resize(const int width, const int height);

	// Called by GUI when a render event occurs.
	void render(sf::RenderWindow &window);
};

}