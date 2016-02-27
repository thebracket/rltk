#include <iostream>
#include "../../rltk/rltk.hpp"
#include <sstream>

bool tick(double duration_ms) {
	auto res = rltk::get_screen_size_px();
	std::stringstream ss;
	ss << "Frame duration: " << duration_ms << " ms (" << (1000.0/duration_ms) << " FPS). Resolution: " << res.first << "x" << res.second;
	rltk::cls_root();
	rltk::print_to_root(1, 1, ss.str());
	return false;
}

int main() {
	rltk::init(tick);
	rltk::run();
}

