/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Retained mode GUI controls
 */
#pragma once

#include "virtual_terminal.hpp"
#include <string>
#include <functional>
#include <boost/optional.hpp>

namespace rltk {

/*
 * Base type for retained-mode GUI controls.
 */
struct gui_control_t {
	virtual void render(virtual_terminal * console)=0;
	virtual bool mouse_in_control(const int tx, const int ty) { return false; }

	// Callbacks
	boost::optional<std::function<void(gui_control_t *)>> on_render_start;
	boost::optional<std::function<void(gui_control_t *)>> on_mouse_over;
};

struct gui_static_text_t : public gui_control_t {
	gui_static_text_t(const int X, const int Y, const std::string txt, const color_t fg, const color_t bg) :
		text(txt), x(X), y(Y), foreground(fg), background(bg) {}

	std::string text = "";
	int x=0;
	int y=0;
	color_t foreground;
	color_t background;

	virtual void render(virtual_terminal * console) override;
	virtual bool mouse_in_control(const int tx, const int ty) override { return (tx >= x and tx <= x + (text.size()) and ty==y); }
};

struct gui_border_box_t : public gui_control_t {
	gui_border_box_t(const bool double_lines, const color_t fg, const color_t bg) : 
		is_double(double_lines), foreground(fg), background(bg) {}
	
	bool is_double = false;
	color_t foreground;
	color_t background;

	virtual void render(virtual_terminal * console) override;
};

}
