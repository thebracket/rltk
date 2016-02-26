#pragma once

#include "types.hpp"
#include <vector>

namespace rltk {
namespace vterm {

class layer {
public:
		

private:
	std::pair<int,int> font_size;
	std::pair<int,int> terminal_size;
	bool render_background = true;
	uint8_t alpha = 255;
	std::vector<char_t> buffer;
};

}
}
