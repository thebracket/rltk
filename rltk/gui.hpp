/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Provides support for complicated GUIs.
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>
#include <memory>

namespace rltk {

struct bounding_box {
	bounding_box() {}
	bounding_box(const int X, const int Y, const int W, const int H) : x(X), y(Y), w(W), h(H) {}
	int x, y, w, h;
};

enum panel_mode { full, fixed_height, fixed_width, expander };

struct panel {
	panel();
	panel(const panel_mode MODE) : mode(MODE) {}

	bounding_box position;
	panel_mode mode;
	std::vector<panel> sub_panels;
	int fix_height;

	void render(panel * parent, sf::Window &window);
	void resize(panel * parent, sf::Window &window);

	void resize_full(panel * parent, sf::Window &window);
	void resize_fixed_height(panel * parent, sf::Window &window);
};

class gui {
public:
	gui();

	inline void on_resized() { window_resized=true; }
	void render(sf::Window &window);
private:
	bool window_resized = true;
	std::vector<panel> panels;
};

}