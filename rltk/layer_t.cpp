#include "layer_t.hpp"

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
		for (auto it=controls.begin(); it != controls.end(); ++it) {
			it->second->render(console.get());
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