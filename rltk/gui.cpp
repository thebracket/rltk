#include "gui.hpp"
#include <stdexcept>
#include <vector>
#include <utility>
#include <algorithm>

namespace rltk {

namespace gui_detail {

std::vector<std::pair<int, layer_t *>> render_order;

}

void gui_t::on_resize(const int w, const int h) {
	screen_width = w;
	screen_height = h;

	for (auto it = layers.begin(); it != layers.end(); ++it) {
		it->second.on_resize(w, h);
	}
}

void gui_t::render(sf::RenderWindow &window) {
	for (auto l : gui_detail::render_order) {
		l.second->render(window);
	}
}

void gui_t::add_layer(const int handle, const int X, const int Y, const int W, const int H, 
	std::string font_name, std::function<void(layer_t *,int,int)> resize_fun, bool has_background,
	int order) 
{
	layers.emplace(std::make_pair(handle, layer_t(X, Y, W, H, font_name, resize_fun, has_background)));
	if (order == -1) {
		order = render_order;
		++render_order;
	}
	gui_detail::render_order.push_back(std::make_pair(order, get_layer(handle)));
	std::sort(gui_detail::render_order.begin(), gui_detail::render_order.end(), 
		[] (std::pair<int, layer_t *> a, std::pair<int, layer_t *> b) 
		{
		return a.first < b.first;
		}
	);
}

void gui_t::delete_layer(const int handle) {
	gui_detail::render_order.erase(std::remove_if(gui_detail::render_order.begin(), gui_detail::render_order.end(), 
		[&handle, this] (std::pair<int, layer_t *> a) 
		{
			if (a.second == this->get_layer(handle)) return true;
			return false;
		}
	), gui_detail::render_order.end());
	layers.erase(handle);
}

layer_t * gui_t::get_layer(const int handle) {
	auto finder = layers.find(handle);
	if (finder == layers.end()) throw std::runtime_error("Unknown layer handle: " + std::to_string(handle));
	return &(finder->second);
}

}
