/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Retained mode GUI controls
 */
#pragma once

#include "virtual_terminal.hpp"

namespace rltk {

/*
 * Base type for retained-mode GUI controls.
 */
struct gui_control_t {
	virtual void render(virtual_terminal * console)=0;
};

}
