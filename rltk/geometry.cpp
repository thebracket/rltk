#include "geometry.hpp"
#include <cstdlib>
#include <cmath>

namespace rltk {

/*
 * From a given point x/y, project forward radius units (generally tiles) at an angle of degrees_radians degrees
 * (in radians).
 */
std::pair<int, int> project_angle(const int x, const int y, const double radius, const double degrees_radians)
{
	return std::make_pair(x + radius * std::cos(degrees_radians), y + radius * std::sin(degrees_radians));
}

/*
 * Perform a function for each line element between x1/y1/z1 and x2/y2/z2. Uses a 3D
 * implementation of Bresenham's line algorithm.
 * https://gist.github.com/yamamushi/5823518
 */
void line_func_3d(int x1, int y1, int z1, const int x2, const int y2, const int z2, std::function<void(int, int, int)> &&func)
{
	float x = x1;
	float y = y1;
	float z = z1;

	float length = distance3d(x1, y1, z1, x2, y2, z2);
	int steps = std::floor(length);
	float x_step = (x - x2) / length;
	float y_step = (y - y2) / length;
	float z_step = (z - z2) / length;

	for (int i=0; i<steps; ++i) {
		x += x_step;
		y += y_step;
		z += z_step;
		func(std::floor(x), std::floor(y), std::floor(z));
	}
}

/*
 * Perform a function for each line element between x1/y1/z1 and x2/y2/z2. Uses a 3D
 * implementation of Bresenham's line algorithm.
 * https://gist.github.com/yamamushi/5823518
 */
void line_func_3d_cancellable(int x1, int y1, int z1, const int x2, const int y2, const int z2, std::function<bool(int, int, int)> &&func)
{
	float x = x1;
	float y = y1;
	float z = z1;

	float length = distance3d(x1, y1, z1, x2, y2, z2);
	int steps = std::floor(length);
	float x_step = (x - x2) / length;
	float y_step = (y - y2) / length;
	float z_step = (z - z2) / length;

	for (int i=0; i<steps; ++i) {
		x += x_step;
		y += y_step;
		z += z_step;
		const bool keep_going = func(std::floor(x), std::floor(y), std::floor(z));
		if (!keep_going) return;
	}
}

}