#include <iostream>
#include "../../rltk/rltk.hpp"

int count = 0;

bool tick() {
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

