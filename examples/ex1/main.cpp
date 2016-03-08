#include "../../rltk/rltk.hpp"

void tick(double time_elapsed) {

}

int main()
{
	rltk::init();
	rltk::register_bitmap_font_load("8x8", "terminal8x8", "../assets/terminal8x8.png", 8, 8);
	rltk::run(tick);

    return 0;
}
