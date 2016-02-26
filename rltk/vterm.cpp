#include "vterm.hpp"
#include <algorithm>
#include "colors.hpp"

using rltk::color::black;

namespace rltk {
namespace vterm {

void layer::resize(const std::pair<int,int> screen_size) {
	const int width = screen_size.first / font_size.first;
	const int height = screen_size.second / font_size.second;
	buffer.resize(height*width);
	cls();
}

void layer::cls() {
	constexpr char_t blank{ black, black, ' ' };
	std::fill(buffer.begin(), buffer.end(), blank);
}

}
}

