#pragma once

#include <functional>

namespace rltk {

using tick_callback_t = std::function<bool(double)>;

}
