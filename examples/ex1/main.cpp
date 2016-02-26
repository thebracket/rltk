#include <iostream>
#include "../../rltk/rltk.hpp"

int count = 0;

bool tick(double duration_ms) {
	std::cout << "Frame duration: " << duration_ms << " ms\n";
	++count;
	if (count > 100) {
		return true;
	} else {
		return false;
	}
}

int main() {
	rltk::init(tick);
	rltk::run();
}

