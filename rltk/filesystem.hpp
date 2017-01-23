/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Minimal filesystem tools
 */
#pragma once

#include <string>
#include <sys/stat.h>

namespace rltk {

inline bool exists(const std::string &filename) noexcept {
    struct stat buffer;
    return (stat (filename.c_str(), &buffer) == 0);
}

}
