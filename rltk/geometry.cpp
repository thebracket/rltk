#include "geometry.hpp"
#include <cstdlib>
#include <cmath>

namespace rltk {

/*
 * From a given point x/y, project forward radius units (generally tiles) at an angle of degrees_radians degrees
 * (in radians).
 */
std::pair<int, int> project_angle(const int &x, const int &y, const double &radius, const double &degrees_radians) noexcept
{
	return std::make_pair(static_cast<int>(x + radius * std::cos(degrees_radians)), static_cast<int>(y + radius * std::sin(degrees_radians)));
}

}