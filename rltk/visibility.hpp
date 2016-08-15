#pragma once
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Provides a wrapper for bitmap fonts.
 */

#include <functional>
#include "geometry.hpp"

namespace rltk {

namespace visibility_private {

/*
 * Benchmark results from example 6 (on my desktop):
 * - Using line_func, it averages 0.3625 uS per line.
 * - Using line_func_cancellable, it averages 0.25 uS per line.
 * - Using a naieve float based slope, it averages 0.2 uS per line.
 */

template<class location_t_, class navigator_t>
void internal_2d_sweep(const location_t_ &position, const int &range, std::function<void(location_t_)> set_visible, 
	std::function<bool(location_t_)> is_opaque, const std::pair<int,int> offset)
{
	bool blocked = false;
	const int start_x = navigator_t::get_x(position);
	const int start_y = navigator_t::get_y(position);
	const int end_x = start_x + offset.first;
	const int end_y = start_y + offset.second;
	
	line_func_cancellable(start_x, start_y, end_x, end_y, [&blocked, &is_opaque, &set_visible, &range, &position] (int X, int Y) {
		if (blocked) return false;
		float distance = distance2d(static_cast<int>(position.x), static_cast<int>(position.y), X, Y);
		if (distance <= range) {
			location_t_ pos = navigator_t::get_xy(X,Y);
			if (!blocked) set_visible(pos);
			if (!is_opaque(pos)) blocked = true;
		}
		return true;
	});
}

}

/* Simple all-direction visibility sweep in 2 dimensions. This requires that your location_t utilize an x and y
 * component. Parameters:
 * position - where you are sweeping from.
 * range - the number of tiles you can traverse.
 * set_visible - a callback (such as bool set_visible(location_t & loc)) to say "this is visible"
 * is_opaque - a callback to ask your map if you can see through a tile.
 *
 * You must provide a navigator_t, just like for path finding. It must support get_x, get_y, and get_xy.
 */
template<class location_t_, class navigator_t>
void visibility_sweep_2d(const location_t_ &position, const int &range, std::function<void(location_t_)> set_visible, 
	std::function<bool(location_t_)> is_opaque)
{
	// You can always see yourself
	set_visible(position);

	// Box-sweep
	for (int i=0-range; i<range; ++i) {
		visibility_private::internal_2d_sweep<location_t_, navigator_t>(position, range, set_visible, is_opaque, std::make_pair(i, 0-range));
		visibility_private::internal_2d_sweep<location_t_, navigator_t>(position, range, set_visible, is_opaque, std::make_pair(i, range));
		visibility_private::internal_2d_sweep<location_t_, navigator_t>(position, range, set_visible, is_opaque, std::make_pair(0-range, i));
		visibility_private::internal_2d_sweep<location_t_, navigator_t>(position, range, set_visible, is_opaque, std::make_pair(range, i));
	}
}

}
