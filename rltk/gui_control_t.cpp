#include "gui_control_t.hpp"

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

}