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
#include <vector>
#include <iostream>

// TODO: Text box, display rich text, list box, combo box, slider, vbar

namespace rltk {

/*
 * Base type for retained-mode GUI controls.
 */
struct gui_control_t {
	virtual void render(virtual_terminal * console)=0;
	virtual bool mouse_in_control(const int tx, const int ty) { return false; }

	// Callbacks
	boost::optional<std::function<void(gui_control_t *)>> on_render_start;
	boost::optional<std::function<void(gui_control_t *, int, int)>> on_mouse_over;
	boost::optional<std::function<void(gui_control_t *, int, int)>> on_mouse_down;
	boost::optional<std::function<void(gui_control_t *, int, int)>> on_mouse_up;
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

struct gui_checkbox_t : public gui_control_t {
	gui_checkbox_t(const int X, const int Y, const bool is_checked, const std::string text, const color_t fg, const color_t bg) :
		checked(is_checked), label(text), foreground(fg), background(bg), x(X), y(Y) {
			on_mouse_down = [] (gui_control_t * ctrl, int tx, int ty) {
				gui_checkbox_t * me = static_cast<gui_checkbox_t *>(ctrl);
				me->click_started = true;
			};
			on_mouse_up = [] (gui_control_t * ctrl, int tx, int ty) {
				gui_checkbox_t * me = static_cast<gui_checkbox_t *>(ctrl);
				if (me->click_started) {
					me->checked = !me->checked;
				}
				me->click_started = false;
			};
		}

	bool checked = false;
	std::string label;
	color_t foreground;
	color_t background;
	int x=0;
	int y=0;
	bool click_started = false;

	virtual void render(virtual_terminal * console) override;
	virtual bool mouse_in_control(const int tx, const int ty) override { 
		return (tx >= x and tx <= x + (label.size()+4) and ty==y); 
	}
};

struct radio {
	bool checked;
	std::string label;
	int value;
};

struct gui_radiobuttons_t : public gui_control_t {
	gui_radiobuttons_t(const int X, const int Y, const std::string heading, const color_t fg, const color_t bg, std::vector<radio> opts) :
		caption(heading), foreground(fg), background(bg), options(opts), x(X), y(Y)
	{
		width = caption.size();
		for (const radio &r : options) {
			if (width < r.label.size()) width = r.label.size();
			if (r.checked) selected_value = r.value;
		}
		height = options.size() + 1;

		on_mouse_down = [] (gui_control_t * ctrl, int tx, int ty) {
			gui_radiobuttons_t * me = static_cast<gui_radiobuttons_t *>(ctrl);
			me->click_started = true;
		};
		on_mouse_up = [] (gui_control_t * ctrl, int tx, int ty) {
			gui_radiobuttons_t * me = static_cast<gui_radiobuttons_t *>(ctrl);
			if (me->click_started) {
				const int option_number = (ty - me->y) -1;
				if (option_number >= 0 and option_number <= me->options.size()) {
					me->selected_value = me->options[option_number].value;
					for (auto &r : me->options) {
						if (r.value == me->selected_value) {
							r.checked = true;
						} else {
							r.checked = false;
						}
					}
				}
			}
			me->click_started = false;
		};
	}

	std::string caption;
	color_t foreground;
	color_t background;
	std::vector<radio> options;
	int x;
	int y;
	int width;
	int height;
	bool click_started = false;
	int selected_value = -1;

	virtual void render(virtual_terminal * console) override;
	virtual bool mouse_in_control(const int tx, const int ty) override { 
		if (tx >= x and tx <= (x + width) and ty >= y and ty <= (y + height)) {
			return true;
		}
		return false;
	}
};

struct gui_hbar_t : public gui_control_t {
	gui_hbar_t(const int X, const int Y, const int W, const int MIN, const int MAX, const int VAL, 
		const color_t FULL_START, const color_t FULL_END, const color_t EMPTY_START, const color_t EMPTY_END,
		const color_t TEXT_COL, std::string PREFIX = "") :
		x(X), y(Y), w(W), min(MIN), max(MAX), value(VAL), full_start(FULL_START), full_end(FULL_END),
		empty_start(EMPTY_START), empty_end(EMPTY_END), text_color(TEXT_COL), prefix(PREFIX)
	{}

	int x;
	int y;
	std::size_t w;
	int min;
	int max;
	int value;
	color_t full_start;
	color_t full_end;
	color_t empty_start;
	color_t empty_end;
	color_t text_color;
	std::string prefix;

	virtual void render(virtual_terminal * console) override;
};

struct gui_vbar_t : public gui_control_t {
	gui_vbar_t(const int X, const int Y, const int H, const int MIN, const int MAX, const int VAL, 
		const color_t FULL_START, const color_t FULL_END, const color_t EMPTY_START, const color_t EMPTY_END,
		const color_t TEXT_COL, std::string PREFIX = "") :
		x(X), y(Y), h(H), min(MIN), max(MAX), value(VAL), full_start(FULL_START), full_end(FULL_END),
		empty_start(EMPTY_START), empty_end(EMPTY_END), text_color(TEXT_COL), prefix(PREFIX)
	{}

	int x;
	int y;
	std::size_t h;
	int min;
	int max;
	int value;
	color_t full_start;
	color_t full_end;
	color_t empty_start;
	color_t empty_end;
	color_t text_color;
	std::string prefix;

	virtual void render(virtual_terminal * console) override;
};

}
