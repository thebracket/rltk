#pragma once

#include <functional>

namespace rltk {

struct color_t {
	color_t() {}
	color_t(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) : red(r), green(g), blue(b), alpha(a) {}

	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

struct char_t {
	color_t color;
	uint8_t glyph; 	
};

using tick_callback_t = std::function<bool(double)>;

}
