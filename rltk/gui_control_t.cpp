#include "gui_control_t.hpp"
#include <sstream>
#include <iostream>

namespace rltk {

void gui_static_text_t::render(virtual_terminal * console) {
	console->print(x, y, text, foreground, background);
}

void gui_border_box_t::render(virtual_terminal * console) {
	console->box(foreground, background, is_double);
}

void gui_checkbox_t::render(virtual_terminal * console) {
	console->set_char(x, y, vchar{'[', foreground, background});
	if (checked) {
		console->set_char(x+1, y, vchar{'X', foreground, background});
	} else {
		console->set_char(x+1, y, vchar{' ', foreground, background});
	}
	console->print(x+2, y, "] " + label, foreground, background);
}

void gui_radiobuttons_t::render(virtual_terminal * console) {
	console->print(x, y, caption, foreground, background);
	int current_y = y+1;
	for (const radio &r : options) {
		console->set_char(x, current_y, vchar{'(', foreground, background});
		if (r.checked) {
			console->set_char(x+1, current_y, vchar{'*', foreground, background});
		} else {
			console->set_char(x+1, current_y, vchar{' ', foreground, background});
		}
		console->print(x+2, current_y, ") " + r.label, foreground, background);
		++current_y;
	}
}

void gui_hbar_t::render(virtual_terminal * console) {
	float fullness = (float)(value - min) / (float)max;
	float full_w_f = fullness * (float)w;
	int full_w = full_w_f;

	std::stringstream ss;
	for (int i=0; i<w; ++i) {
		ss << ' ';
	}
	std::string s = ss.str();

	std::stringstream display;
	display << prefix << value << "/" << max;
	std::string tmp = display.str();
	const int start = (w/2) - (tmp.size() / 2);
	for (int i=0; i<tmp.size(); ++i) {
		s[i + start] = tmp[i];
	}

	for (int i=0; i<w; ++i) {
		const float pct = (float)i / (float)w;
		if (i <= full_w) {
			console->set_char(x+i, y, vchar{s[i], text_color, lerp(full_start, full_end, pct)});
		} else {
			console->set_char(x+i, y, vchar{s[i], text_color, lerp(empty_start, empty_end, pct)});
		}
	}
}

}