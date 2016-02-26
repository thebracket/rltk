#include "vterm.hpp"
#include <algorithm>
#include "colors.hpp"

using rltk::color::black;

namespace rltk {
namespace vterm {

void layer::resize(const std::pair<int,int> screen_size) {
	const int width = screen_size.first / font_size.first;
	const int height = screen_size.second / font_size.second;
	terminal_size = std::make_pair(width,height);
	buffer.resize(height*width);
	cls();
}

void layer::cls() {
	constexpr char_t blank{ black, black, 'A' };
	std::fill(buffer.begin(), buffer.end(), blank);
}

void layer::print(const int idx, const std::string text, const color_t fg, const color_t bg) {
	for (std::size_t i=0; i<text.size(); ++i) {
		const char target = text[i];
		const char_t render{ fg, bg, static_cast<uint8_t>(target) };
		buffer[idx] = render;
	}
}

}
}

