/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Layer type used by the GUI
 */
#pragma once

#include "virtual_terminal.hpp"
#include "virtual_terminal_sparse.hpp"
#include "gui_control_t.hpp"
#include <unordered_map>

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

	/* This specialization is for sparse consoles */
	layer_t(bool sparse, const int X, const int Y, const int W, const int H, std::string font_name, std::function<void(layer_t *,int,int)> resize_fun) :
		x(X), y(Y), w(W), h(H), font(font_name), resize_func(resize_fun)
	{
		// Sparse is unusued, but is there to differentiate the signature.
		sconsole = std::make_unique<virtual_terminal_sparse>(font_name, x, y);
	    sconsole->resize_pixels(w, h);
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

	// If it has a sparse console, this is it.
	std::unique_ptr<virtual_terminal_sparse> sconsole;

	// If retained-mode controls are present, they are in here.
	std::unordered_map<int, std::unique_ptr<gui_control_t>> controls;

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

	// Retained Mode Controls
	template<class T>
	T * control(const int handle) { 
		auto finder = controls.find(handle);
		if (finder == controls.end()) throw std::runtime_error("Unknown GUI control handle: " + std::to_string(handle));
		return static_cast<T *>(finder->second.get());
	}

	gui_control_t * control(const int handle) {
		auto finder = controls.find(handle);
		if (finder == controls.end()) throw std::runtime_error("Unknown GUI control handle: " + std::to_string(handle));
		return finder->second.get();
	}

	inline void remove_control(const int handle) {
		controls.erase(handle);
	}

	inline void check_handle_uniqueness(const int handle) {
		auto finder = controls.find(handle);
		if (finder != controls.end()) throw std::runtime_error("Adding a duplicate control handle: " + std::to_string(handle));
	}

	inline void add_static_text(const int handle, const int x, const int y, const std::string text, const color_t fg, const color_t bg) {
		check_handle_uniqueness(handle);
		controls.emplace(handle, std::make_unique<gui_static_text_t>(x, y, text, fg, bg));
	}

	inline void add_boundary_box(const int handle, const bool double_lines, const color_t fg, const color_t bg) {
		check_handle_uniqueness(handle);
		controls.emplace(handle, std::make_unique<gui_border_box_t>(double_lines, fg, bg));
	}

	inline void add_checkbox(const int handle, const int x, const int y, const std::string label, const bool checked, const color_t fg, const color_t bg) {
		check_handle_uniqueness(handle);
		controls.emplace(handle, std::make_unique<gui_checkbox_t>(x, y, checked, label, fg, bg));
	}

	inline void add_radioset(const int handle, const int x, const int y, const std::string caption, const color_t fg, const color_t bg, std::vector<radio> opts) {
		check_handle_uniqueness(handle);
		controls.emplace(handle, std::make_unique<gui_radiobuttons_t>(x, y, caption, fg, bg, opts));
	}

	inline void add_hbar(const int handle, const int X, const int Y, const int W, const int MIN, const int MAX, const int VAL, 
		const color_t FULL_START, const color_t FULL_END, const color_t EMPTY_START, const color_t EMPTY_END, const color_t TEXT_COL, const std::string prefix="")
	{
		check_handle_uniqueness(handle);
		controls.emplace(handle, std::make_unique<gui_hbar_t>(X, Y, W, MIN, MAX, VAL, FULL_START, FULL_END, EMPTY_START, EMPTY_END, TEXT_COL, prefix));
	}

	inline void add_vbar(const int handle, const int X, const int Y, const int H, const int MIN, const int MAX, const int VAL, 
		const color_t FULL_START, const color_t FULL_END, const color_t EMPTY_START, const color_t EMPTY_END, const color_t TEXT_COL, const std::string prefix="")
	{
		check_handle_uniqueness(handle);
		controls.emplace(handle, std::make_unique<gui_vbar_t>(X, Y, H, MIN, MAX, VAL, FULL_START, FULL_END, EMPTY_START, EMPTY_END, TEXT_COL, prefix));
	}

	inline void add_listbox(const int handle, const int X, const int Y, const int VAL, std::vector<list_item> options, std::string label,
		const color_t label_fg, const color_t label_bg, const color_t ITEM_FG, const color_t ITEM_BG,
		const color_t sel_fg, const color_t sel_bg)
	{
		check_handle_uniqueness(handle);
		controls.emplace(handle, std::make_unique<gui_listbox_t>(X, Y, VAL, options, label, label_fg, label_bg, ITEM_FG, ITEM_BG, sel_fg, sel_bg));
	}
};

}