#pragma once

#include <functional>

namespace rltk {

struct color_t {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

struct char_t {
	color_t fg;
	color_t bg;
	uint8_t glyph; 	
};

using tick_callback_t = std::function<bool(double)>;

}
