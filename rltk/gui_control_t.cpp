#include "gui_control_t.hpp"

namespace rltk {

void gui_static_text_t::render(virtual_terminal * console) {
	console->print(x, y, text, foreground, background);
}

void gui_border_box_t::render(virtual_terminal * console) {
	console->box(foreground, background, is_double);
}

}