/* RLTK (RogueLike Tool Kit) 1.00
* Copyright (c) 2016-Present, Bracket Productions.
* Licensed under the LGPL - see LICENSE file.
*/

#include <string>
#include <functional>

namespace rltk {
void run(std::function<void()> on_tick, const int window_width=1024, const int window_height=768, const std::string window_title="RLTK Roguelike");
}
