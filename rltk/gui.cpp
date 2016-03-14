#include "gui.hpp"
#include <stdexcept>

namespace rltk {

gui::gui() {

}

void gui::render(sf::Window &window) {
	if (window_resized) {
		window_resized = false;
		for (panel &p : panels) p.resize(nullptr, window);
	}
	for (panel &p : panels) {
		p.render(nullptr, window);
	}
}

void panel::resize_full(panel * parent, sf::Window &window) {
	// We want to take up the whole available area
	int x, y, w, h;

	if (parent != nullptr) {
		x = parent->position.x;
		y = parent->position.y;
		w = parent->position.w;
		h = parent->position.h;
	} else {
		x = 0;
		y = 0;
		sf::Vector2u window_size = window.getSize();
		w = window_size.x;
		h = window_size.y;
	}

	position = {x,y,w,h};

	for (panel &child : sub_panels) {
		child.resize(this, window);
	}
}

void panel::resize_fixed_height(panel * parent, sf::Window &window) {
	int x, y, w, h;
	if (parent != nullptr) {
		// Need to figure out what to do here!
	} else {
		sf::Vector2u window_size = window.getSize();
		x = 0;
		y = 0;
		w = window_size.x;
		h = fix_height;
	}
	position = {x, y, w, h};

	const int num_children = sub_panels.size();
	const int fair_distribution_x = w / num_children;
	int allocated = 0;

	for (panel &child : sub_panels) {
		child.resize(this, window);
	}
}

void panel::resize(panel * parent, sf::Window &window) {
	switch (mode) {
	case full : resize_full(parent, window); break;
	case fixed_height : resize_fixed_height(parent, window); break;
	default : throw std::runtime_error("Unimplemented layout function!");
	}
}

void panel::render(panel * parent, sf::Window &window) {

}

}