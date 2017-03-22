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
std::pair<int, int> project_angle(const int &x, const int &y, const double &radius, const double &degrees_radians) noexcept;

/*
 * Provides a correct 2D distance between two points.
 */
inline float distance2d(const int &x1, const int &y1, const int &x2, const int &y2) noexcept {
	const float dx = (float)x1 - (float)x2;
    const float dy = (float)y1 - (float)y2;
    return std::sqrt((dx*dx) + (dy*dy));
}

/*
 * Provides a fast 2D distance between two points, omitting the square-root; compare
 * with other squared distances.
 */
inline float distance2d_squared(const int &x1, const int &y1, const int &x2, const int &y2) noexcept {
    const float dx = (float)x1 - (float)x2;
    const float dy = (float)y1 - (float)y2;
    return (dx*dx) + (dy*dy);
}

/*
 * Provides 2D Manhattan distance between two points.
 */
inline float distance2d_manhattan(const int &x1, const int &y1, const int &x2, const int &y2) noexcept {
    const float dx = (float)x1 - (float)x2;
    const float dy = (float)y1 - (float)y2;
    return std::abs(dx) + std::abs(dy);
}

/*
 * Provides a correct 3D distance between two points.
 */
inline float distance3d(const int &x1, const int &y1, const int &z1, const int &x2, const int &y2, const int &z2) noexcept
{
	const float dx = (float)x1 - (float)x2;
    const float dy = (float)y1 - (float)y2;
    const float dz = (float)z1 - (float)z2;
    return std::sqrt((dx*dx) + (dy*dy) + (dz*dz));
}

/*
 * Provides a fast 3D distance between two points, omitting the square-root; compare
 * with other squared distances.
 */
inline float distance3d_squared(const int &x1, const int &y1, const int &z1, const int &x2, const int &y2, const int &z2) noexcept
{
    float dx = (float)x1 - (float)x2;
    float dy = (float)y1 - (float)y2;
    float dz = (float)z1 - (float)z2;
    return (dx*dx) + (dy*dy) + (dz*dz);
}

/*
 * Provides Manhattan distance between two 3D points.
 */
inline float distance3d_manhattan(const int &x1, const int &y1, const int &z1, const int &x2, const int &y2, const int &z2) noexcept
{
    const float dx = (float)x1 - (float)x2;
    const float dy = (float)y1 - (float)y2;
    const float dz = (float)z1 - (float)z2;
    return std::abs(dx) + std::abs(dy) + std::abs(dz);
}

/*
 * Perform a function for each line element between x1/y1 and x2/y2. We used to use Bresenham's line,
 * but benchmarking showed a simple float-based setup to be faster.
 */
inline void line_func(const int &x1, const int &y1, const int &x2, const int &y2, std::function<void(int, int)> &&func) noexcept
{
	float x = static_cast<float>(x1) + 0.5F;
    float y = static_cast<float>(y1) + 0.5F;
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
template <typename F>
void line_func_3d(const int &x1, const int &y1, const int &z1, const int &x2, const int &y2, const int &z2, F &&func) noexcept
{
    float x = static_cast<float>(x1)+0.5F;
    float y = static_cast<float>(y1)+0.5F;
    float z = static_cast<float>(z1)+0.5F;

    float length = distance3d(x1, y1, z1, x2, y2, z2);
    int steps = static_cast<int>(std::floor(length));
    float x_step = (x - x2) / length;
    float y_step = (y - y2) / length;
    float z_step = (z - z2) / length;

    for (int i=0; i<steps; ++i) {
        x += x_step;
        y += y_step;
        z += z_step;
        func(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)), static_cast<int>(std::floor(z)));
    }
}

/*
 * Perform a function for each line element between x1/y1 and x2/y2. We used to use Bresenham's algorithm,
 * but benchmarking showed that a simple float based vector was faster.
 */
template <typename F>
inline void line_func_cancellable(const int &x1, const int &y1, const int &x2, const int &y2, F &&func) noexcept {
    float x = static_cast<float>(x1) + 0.5F;
    float y = static_cast<float>(y1) + 0.5F;
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
 */
template<typename F>
void line_func_3d_cancellable(const int &x1, const int &y1, const int &z1, const int &x2, const int &y2, const int &z2, F &&func) noexcept
{
    float x = static_cast<float>(x1)+0.5F;
    float y = static_cast<float>(y1)+0.5F;
    float z = static_cast<float>(z1)+0.5F;

    float length = distance3d(x1, y1, z1, x2, y2, z2);
    int steps = static_cast<int>(std::floor(length));
    float x_step = (x - x2) / length;
    float y_step = (y - y2) / length;
    float z_step = (z - z2) / length;

    for (int i=0; i<steps; ++i) {
        x += x_step;
        y += y_step;
        z += z_step;
        const bool keep_going = func(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)), static_cast<int>(std::floor(z)));
        if (!keep_going) return;
    }
}
}