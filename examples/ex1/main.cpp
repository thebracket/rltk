#include <iostream>
#include "../../rltk/rltk.hpp"

int main() {
	rltk::internal::sdl2 sdl;
	int n=0;
	while (n < 1000) {
		++n;
		sdl.clear_to_black();
		sdl.present();
	}
}

