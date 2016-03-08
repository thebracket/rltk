#include "geometry.hpp"
#include <cstdlib>
#include <cmath>

namespace rltk {

constexpr double DEGRAD = 3.14159 / 180;

/*
 * From a given point x/y, project forward radius units (generally tiles) at an angle of degrees_radians degrees
 * (in radians).
 */
std::pair<int, int> project_angle(const int x, const int y, const double radius, const double degrees_radians)
{
	return std::make_pair(x + radius * std::cos(degrees_radians), y + radius * std::sin(degrees_radians));
}

/*
 * Perform a function for each line element between x1/y1 and x2/y2. Uses Bresenham's
 * algorithm; initial implementation from
 * http://www.roguebasin.com/index.php?title=Bresenham%27s_Line_Algorithm
 */
void line_func(int x1, int y1, const int x2, const int y2, std::function<void(int, int)> func)
{
	int delta_x(x2 - x1);
	signed char const ix((delta_x > 0) - (delta_x < 0));
	delta_x = std::abs(delta_x) << 1;

	int delta_y(y2 - y1);
	signed char const iy((delta_y > 0) - (delta_y < 0));
	delta_y = std::abs(delta_y) << 1;

	func(x1, y1);

	if (delta_x >= delta_y)
	{
		// error may go below zero
		int error(delta_y - (delta_x >> 1));

		while (x1 != x2)
		{
			if ((error >= 0) && (error || (ix > 0)))
			{
				error -= delta_x;
				y1 += iy;
			}
			// else do nothing

			error += delta_y;
			x1 += ix;

			func(x1, y1);
		}
	}
	else
	{
		// error may go below zero
		int error(delta_x - (delta_y >> 1));

		while (y1 != y2)
		{
			if ((error >= 0) && (error || (iy > 0)))
			{
				error -= delta_y;
				x1 += ix;
			}
			// else do nothing

			error += delta_x;
			y1 += iy;

			func(x1, y1);
		}
	}
}

/*
 * Perform a function for each line element between x1/y1/z1 and x2/y2/z2. Uses a 3D
 * implementation of Bresenham's line algorithm.
 * https://gist.github.com/yamamushi/5823518
 */
void line_func_3d(int x1, int y1, int z1, const int x2, const int y2, const int z2, std::function<void(int, int, int)> func)
{
	int i, dx, dy, dz, l, m, n, x_inc, y_inc, z_inc, err_1, err_2, dx2, dy2, dz2;
	int point[3];

	point[0] = x1;
	point[1] = y1;
	point[2] = z1;
	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;
	x_inc = (dx < 0) ? -1 : 1;
	l = abs(dx);
	y_inc = (dy < 0) ? -1 : 1;
	m = abs(dy);
	z_inc = (dz < 0) ? -1 : 1;
	n = abs(dz);
	dx2 = l << 1;
	dy2 = m << 1;
	dz2 = n << 1;

	if ((l >= m) && (l >= n))
	{
		err_1 = dy2 - l;
		err_2 = dz2 - l;
		for (i = 0; i < l; i++)
		{
			func(point[0], point[1], point[2]);
			if (err_1 > 0)
			{
				point[1] += y_inc;
				err_1 -= dx2;
			}
			if (err_2 > 0)
			{
				point[2] += z_inc;
				err_2 -= dx2;
			}
			err_1 += dy2;
			err_2 += dz2;
			point[0] += x_inc;
		}
	}
	else if ((m >= l) && (m >= n))
	{
		err_1 = dx2 - m;
		err_2 = dz2 - m;
		for (i = 0; i < m; i++)
		{
			func(point[0], point[1], point[2]);
			if (err_1 > 0)
			{
				point[0] += x_inc;
				err_1 -= dy2;
			}
			if (err_2 > 0)
			{
				point[2] += z_inc;
				err_2 -= dy2;
			}
			err_1 += dx2;
			err_2 += dz2;
			point[1] += y_inc;
		}
	}
	else
	{
		err_1 = dy2 - n;
		err_2 = dx2 - n;
		for (i = 0; i < n; i++)
		{
			func(point[0], point[1], point[2]);
			if (err_1 > 0)
			{
				point[1] += y_inc;
				err_1 -= dz2;
			}
			if (err_2 > 0)
			{
				point[0] += x_inc;
				err_2 -= dz2;
			}
			err_1 += dy2;
			err_2 += dx2;
			point[2] += z_inc;
		}
	}
	func(point[0], point[1], point[2]);
}


}