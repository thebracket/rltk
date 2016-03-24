/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Example 7: Introduction to complex GUIs
 */

// You need to include the RLTK header
#include "../../rltk/rltk.hpp"

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
using namespace rltk::colors;

// Tick is called every frame. The parameter specifies how many ms have elapsed
// since the last time it was called.
void tick(double duration_ms) {
}

// Your main function
int main()
{
	// This time, we're using a full initialization: width, height, window title, and "false" meaning we don't
	// want an automatically generated root console. This is necessary when you want to use the complex layout
	// functions.
	init(1024, 768, "RLTK Complex GUI", false);
	register_bitmap_font("8x8", "../assets/terminal8x8.png", 8, 8);
	run(tick, "8x8");

    return 0;
}
