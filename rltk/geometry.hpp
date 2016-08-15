#pragma once

/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
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
 * Provides a correct 2D distance between two points.
 */
inline float distance2d(int x1, int y1, int x2, int y2) {
	float dx = (float)x1 - (float)x2;
    float dy = (float)y1 - (float)y2;
    return std::sqrt((dx*dx) + (dy*dy));
}

/*
 * Provides a fast 2D distance between two points, omitting the square-root; compare
 * with other squared distances.
 */
inline float distance2d_squared(int x1, int y1, int x2, int y2) {
    float dx = (float)x1 - (float)x2;
    float dy = (float)y1 - (float)y2;
    return (dx*dx) + (dy*dy);
}

/*
 * Provides 2D Manhattan distance between two points.
 */
inline float distance2d_manhattan(int x1, int y1, int x2, int y2) {
    float dx = (float)x1 - (float)x2;
    float dy = (float)y1 - (float)y2;
    return std::abs(dx) + std::abs(dy);
}

/*
 * Provides a correct 3D distance between two points.
 */
inline float distance3d(int x1, int y1, int z1, int x2,	int y2, int z2)
{
	float dx = (float)x1 - (float)x2;
    float dy = (float)y1 - (float)y2;
    float dz = (float)z1 - (float)z2;
    return std::sqrt((dx*dx) + (dy*dy) + (dz*dz));
}

/*
 * Provides a fast 3D distance between two points, omitting the square-root; compare
 * with other squared distances.
 */
inline float distance3d_squared(int x1, int y1, int z1, int x2,	int y2, int z2)
{
    float dx = (float)x1 - (float)x2;
    float dy = (float)y1 - (float)y2;
    float dz = (float)z1 - (float)z2;
    return (dx*dx) + (dy*dy) + (dz*dz);
}

/*
 * Provides Manhattan distance between two 3D points.
 */
inline float distance3d_manhattan(int x1, int y1, int z1, int x2, int y2, int z2)
{
    float dx = (float)x1 - (float)x2;
    float dy = (float)y1 - (float)y2;
    float dz = (float)z1 - (float)z2;
    return std::abs(dx) + std::abs(dy) + std::abs(dz);
}

/*
 * Perform a function for each line element between x1/y1 and x2/y2. We used to use Bresenham's line,
 * but benchmarking showed a simple float-based setup to be faster.
 */
inline void line_func(int x1, int y1, const int x2, const int y2, std::function<void(int, int)> &&func)
{
	float x = static_cast<float>(x1);
    float y = static_cast<float>(y1);
    const float dest_x = static_cast<float>(x2);
    const float dest_y = static_cast<float>(y2);
    const float n_steps = distance2d(x1,y1,x2,y2);
    const int steps = static_cast<const int>(std::floor(n_steps) + 1);
    const float slope_x = (dest_x - x) / n_steps;
    const float slope_y = (dest_y - y) / n_steps;

    for (int i = 0; i < steps; ++i) {
        func(static_cast<int>(x), static_cast<int>(y));
        x += slope_x;
        y += slope_y;
    }
}

/*
 * Perform a function for each line element between x1/y1/z1 and x2/y2/z2. Uses a 3D
 * implementation of Bresenham's line algorithm.
 * https://gist.github.com/yamamushi/5823518
 */
void line_func_3d(int x1, int y1, int z1, const int x2, const int y2, const int z2, std::function<void(int, int, int)> &&func);

/*
 * Perform a function for each line element between x1/y1 and x2/y2. We used to use Bresenham's algorithm,
 * but benchmarking showed that a simple float based vector was faster.
 */
inline void line_func_cancellable(int x1, int y1, const int x2, const int y2, std::function<bool(int, int)> &&func) {
    float x = static_cast<float>(x1);
    float y = static_cast<float>(y1);
    const float dest_x = static_cast<float>(x2);
    const float dest_y = static_cast<float>(y2);
    const float n_steps = distance2d(x1,y1,x2,y2);
    const int steps = static_cast<const int>(std::floor(n_steps) + 1);
    const float slope_x = (dest_x - x) / n_steps;
    const float slope_y = (dest_y - y) / n_steps;

    for (int i = 0; i < steps; ++i) {
        if (!func(static_cast<int>(x), static_cast<int>(y))) return;
        x += slope_x;
        y += slope_y;
    }
}

/*
 * Perform a function for each line element between x1/y1/z1 and x2/y2/z2. Uses a 3D
 * implementation of Bresenham's line algorithm.
 * https://gist.github.com/yamamushi/5823518
 * Cancellable version: the function can return "false" to stop traversing the line.
 */
void line_func_3d_cancellable(int x1, int y1, int z1, const int x2, const int y2, const int z2, std::function<bool(int, int, int)> &&func);

}