#include "gui.hpp"
#include <stdexcept>
#include <utility>
#include <algorithm>

namespace rltk {

namespace gui_detail {

std::vector<std::pair<int, layer_t *>> render_order;

}

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
		for (gui_control_t &control : controls) {
			control.render(console.get());
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

void gui_t::add_owner_layer(const int handle, const int X, const int Y, const int W, const int H, 
	std::function<void(layer_t *,int,int)> resize_fun, std::function<void(layer_t *, sf::RenderTexture &)> owner_draw_fun, int order) 
{
	layers.emplace(std::make_pair(handle, layer_t(X, Y, W, H, resize_fun, owner_draw_fun)));
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
