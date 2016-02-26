#include <iostream>
#include "../../rltk/rltk.hpp"

bool tick(double duration_ms) {
	auto res = rltk::get_screen_size_px();
	std::cout << "Frame duration: " << duration_ms << " ms. Resolution: " << res.first << "x" << res.second << "\n";
	return false;
}

int main() {
	rltk::init(tick);
	rltk::run();
}

