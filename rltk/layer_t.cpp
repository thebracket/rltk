#include "layer_t.hpp"
#include "input_handler.hpp"

namespace rltk {

void layer_t::make_owner_draw_backing() {
	if (!backing) {
		backing = std::make_unique<sf::RenderTexture>();
	}
	backing->create(w, h);
}

void layer_t::on_resize(const int width, const int height) {
	resize_func(this, width, height);
	if (console) {
		if (console->visible) {
			console->set_offset(x,y);
			console->resize_pixels(w, h);
		} else {
			make_owner_draw_backing();
		}
	}
}

void layer_t::render(sf::RenderWindow &window) {
	if (console) {
		if (!controls.empty()) {

			// Render start events
			for (auto it=controls.begin(); it != controls.end(); ++it) {
				if (it->second->on_render_start) {
					auto callfunc = it->second->on_render_start.get();
					callfunc(it->second.get());
				}
			}

			int mouse_x, mouse_y;
			std::tie(mouse_x, mouse_y) = get_mouse_position();

			if (mouse_x >= x and mouse_x <= (x+w) and mouse_y >= y and mouse_y <= (y+h)) {
				// Mouse over in here is possible.
				auto font_dimensions = console->get_font_size();
				const int terminal_x = (mouse_x - x) / font_dimensions.first;
				const int terminal_y = (mouse_y - y) / font_dimensions.second;

				for (auto it=controls.begin(); it != controls.end(); ++it) {
					// Mouse over
					if (it->second->mouse_in_control(terminal_x, terminal_y)) {
						if (it->second->on_mouse_over) {
							auto callfunc = it->second->on_mouse_over.get();
							callfunc(it->second.get());
						}
					}

					// Mouse down and up
					if (get_mouse_button_state(button::LEFT) and it->second->on_mouse_down) {
						auto callfunc = it->second->on_mouse_down.get();
						callfunc(it->second.get());
					}
					if (!get_mouse_button_state(button::LEFT) and it->second->on_mouse_up) {
						auto callfunc = it->second->on_mouse_up.get();
						callfunc(it->second.get());
					}
				}
			}

			for (auto it=controls.begin(); it != controls.end(); ++it) {
				it->second->render(console.get());
			}
		}
		console->render(window);
	} else {
		if (!backing) make_owner_draw_backing();
		backing->clear(sf::Color(0,0,0,0));
		owner_draw_func(this, *backing);
		backing->display();
		sf::Sprite compositor(backing->getTexture());
		compositor.move(x, y);
		window.draw(sf::Sprite(compositor));
	}
}

}