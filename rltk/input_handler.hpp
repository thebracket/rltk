#pragma once
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Class for providing a simple input interface.
 * Provides functions that can be queried for the current state of
 * the input system.
 *
 */

#include <utility>
#include <SFML/Graphics.hpp>
#include "ecs.hpp"

namespace rltk {

/* Helper constants to represent mouse buttons */
namespace button {
constexpr int LEFT = 0;
constexpr int RIGHT = 1;
constexpr int MIDDLE = 2;
constexpr int SIDE1 = 3;
constexpr int SIDE2 = 4;
constexpr int WHEEL_UP = 5;
constexpr int WHEEL_DOWN = 6;
}

/* Does the game window currently have focus? You might want to pause if it doesn't. */
extern bool is_window_focused();

/* Setter function for window focus */
extern void set_window_focus_state(const bool &s);

/* Mouse state reset: clears all mouse state */
extern void reset_mouse_state();

/* Update the stored mouse position. Does not actually move the mouse. */
extern void set_mouse_position(const int x, const int y);
extern std::pair<int,int> get_mouse_position();

/* Mouse button state */
extern void set_mouse_button_state(const int button, const bool state);
extern bool get_mouse_button_state(const int button);

/* Keyboard queue */
extern void enqueue_key_pressed(sf::Event &event);

struct key_pressed_t : base_message_t {
public:
    key_pressed_t() {}
    key_pressed_t(sf::Event ev) : event(ev) {}
    sf::Event event;
};

}
