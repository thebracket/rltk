#pragma once

/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Random number generator class.
 */

#include <functional>
#include <utility>
#include <cmath>

namespace rltk {

/*
 * From a given point x/y, project forward radius units (generally tiles) at an angle of degrees_radians degrees
 * (in radians).
 */
std::pair<int, int> project_angle(const int x, const int y, const double radius, const double degrees_radians);

/*
 * Perform a function for each line element between x1/y1 and x2/y2. Uses Bresenham's
 * algorithm; initial implementation from
 * http://www.roguebasin.com/index.php?title=Bresenham%27s_Line_Algorithm
 */
void line_func(int x1, int y1, const int x2, const int y2, std::function<void(int, int)> func);

/*
 * Perform a function for each line element between x1/y1/z1 and x2/y2/z2. Uses a 3D
 * implementation of Bresenham's line algorithm.
 * https://gist.github.com/yamamushi/5823518
 */
void line_func_3d(int x1, int y1, int z1, const int x2, const int y2, const int z2, std::function<void(int, int, int)> func);

/*
 * Provides a correct 3D distance between two points.
 */
inline float distance3d(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2,	uint8_t y2, uint8_t z2)
{
	return std::sqrt(std::pow(x1 - x2, 2.0) + std::pow(y1 - y2, 2.0)+ std::pow(z1 - z2, 2.0));
}

/*
 * Provides a fast 3D distance between two points, omitting the square-root; compare
 * with other squared distances.
 */
inline float distance3d_squared(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2,	uint8_t y2, uint8_t z2)
{
	return std::pow(x1 - x2, 2.0) + std::pow(y1 - y2, 2.0) + std::pow(z1 - z2, 2.0);
}

}