#include "input_handler.hpp"
#include "scaling.hpp"
#include <array>
#include <algorithm>

namespace rltk {

namespace state {
bool window_focused = true;
std::array<bool, 7> mouse_button_pressed;
int mouse_x = 0;
int mouse_y = 0;
}

bool is_window_focused() {
	return rltk::state::window_focused;
}

void set_window_focus_state(const bool &s) {
	rltk::state::window_focused = s;
}

void reset_mouse_state() {
	std::fill(rltk::state::mouse_button_pressed.begin(), rltk::state::mouse_button_pressed.end(), false);
	rltk::state::mouse_x = 0;
	rltk::state::mouse_y = 0;
}

void set_mouse_button_state(const int button, const bool state) {
	rltk::state::mouse_button_pressed[button] = state;
}

bool get_mouse_button_state(const int button) {
	return rltk::state::mouse_button_pressed[button];
}

void set_mouse_position(const int x, const int y) {
	rltk::state::mouse_x = static_cast<int>(x / scale_factor);
	rltk::state::mouse_y = static_cast<int>(y / scale_factor);
}

std::pair<int,int> get_mouse_position() {
	return std::make_pair(rltk::state::mouse_x, rltk::state::mouse_y);
}

void enqueue_key_pressed(sf::Event &event) {
	emit(key_pressed_t{event});
}

}