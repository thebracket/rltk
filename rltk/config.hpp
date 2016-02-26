#pragma once

#include <string>
#include <functional>

namespace rltk {
namespace config {

extern std::string window_title;
extern int initial_height;
extern int initial_width;
extern std::function<bool()> main_callback;

}
}
