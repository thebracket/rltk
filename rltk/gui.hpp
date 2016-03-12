/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Provides support for complicated GUIs.
 */

#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

namespace rltk {

struct bounding_box {
	bounding_box() {}
	bounding_box(const int X, const int Y, const int W, const int H) : x(X), y(Y), w(W), h(H) {}
	int x, y, w, h;
};

class panel {
public:
	bounding_box position;
};

class gui {
public:
	gui();

	inline void on_resized() { window_resized=true; }
	void render(sf::Window &window);
private:
	bool window_resized = true;
};

}