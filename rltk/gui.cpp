#include "gui.hpp"
#include <stdexcept>
#include <vector>

namespace rltk {

namespace gui_detail {

std::vector<layer_t *> render_order;

}

void gui_t::on_resize(const int w, const int h) {
	screen_width = w;
	screen_height = h;

	for (auto it = layers.begin(); it != layers.end(); ++it) {
		it->second.on_resize(w, h);
	}
}

void gui_t::render(sf::RenderWindow &window) {
	for (layer_t * l : gui_detail::render_order) {
		l->render(window);
	}
}

void gui_t::add_layer(const int handle, const int X, const int Y, const int W, const int H, 
	std::string font_name, std::function<void(layer_t *,int,int)> resize_fun, bool has_background) 
{
	layers.emplace(std::make_pair(handle, layer_t(X, Y, W, H, font_name, resize_fun, has_background)));
	gui_detail::render_order.push_back(get_layer(handle));
}

layer_t * gui_t::get_layer(const int handle) {
	auto finder = layers.find(handle);
	if (finder == layers.end()) throw std::runtime_error("Unknown layer handle: " + std::to_string(handle));
	return &(finder->second);
}

}
