#pragma once

#include "types.hpp"
#include <vector>

namespace rltk {
namespace vterm {

class layer {
public:
	void resize(const std::pair<int,int> screen_size);
	void cls();
	inline int idx(const int x, const int y) { return (y*terminal_size.first)+x;  }
	inline void set(const int idx, const char_t target) { buffer[idx] = target; }
	inline void set(const int x, const int y, const char_t target) { set(idx(x,y),target);  }
	void print(const int idx, const std::string text, const color_t fg, const color_t bg);
	inline void print(const int x, const int y, const std::string text, const color_t fg, const color_t bg) { print(idx(x,y), text, fg, bg); };

	std::pair<int,int> font_size{8,8};
	std::pair<int,int> terminal_size;
	bool render_background = true;
	uint8_t alpha = 255;
	std::vector<char_t> buffer;
};

}
}
