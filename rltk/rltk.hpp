#pragma once

/*
 * RLTK - Copyriht (c) 2016, Bracket Productions. This is licensed under the GNU Lesser GPL - see the LICENSE file for details.
*/

#include <string>
#include <functional>

namespace rltk {

void init(std::function<bool()> tick_handler, const int width = 800, const int height = 600, const std::string title = "RLTK Window");
void run();

}

