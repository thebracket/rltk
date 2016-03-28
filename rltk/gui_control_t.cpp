#include "gui_control_t.hpp"
#include <sstream>
#include <algorithm>

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
	for (int i=0; i < std::min(tmp.size(),w); ++i) {
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

void gui_vbar_t::render(virtual_terminal * console) {
	float fullness = (float)(value - min) / (float)max;
	float full_h_f = fullness * (float)h;
	int full_h = full_h_f;

	std::stringstream ss;
	for (int i=0; i<h; ++i) {
		ss << ' ';
	}
	std::string s = ss.str();
	
	std::stringstream display;
	display << prefix << value << "/" << max;
	std::string tmp = display.str();
	const int start = (h/2) - (tmp.size() / 2);
	for (int i=0; i < std::min(tmp.size(), h); ++i) {
		s[i + start] = tmp[i];
	}

	for (int i=0; i<h; ++i) {
		const float pct = (float)i / (float)h;
		if (i <= full_h) {
			console->set_char(x, y+i, vchar{s[i], text_color, lerp(full_start, full_end, pct)});
		} else {
			console->set_char(x, y+i, vchar{s[i], text_color, lerp(empty_start, empty_end, pct)});
		}
	}
}

void gui_listbox_t::render(virtual_terminal * console) {
	console->box(x, y, w+3, items.size()+1, caption_fg, caption_bg, false);
	console->print(x+3, y, caption, caption_fg, caption_bg);
	console->set_char(x+1, y, vchar{180, caption_fg, caption_bg});
	console->set_char(x+2, y, vchar{' ', caption_fg, caption_bg});
	console->set_char(x+w, y, vchar{' ', caption_fg, caption_bg});
	console->set_char(x+w+1, y, vchar{195, caption_fg, caption_bg});
	int current_y = y+1;
	for (const list_item &item : items) {
		if (item.value == selected_value) {
			console->print(x+2, current_y, item.label, selected_fg, selected_bg);
		} else {
			console->print(x+2, current_y, item.label, item_fg, item_bg);			
		}
		++current_y;
	}
}

}