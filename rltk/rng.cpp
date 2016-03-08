#include "rng.hpp"

#include <iostream>

using std::mt19937;
using std::uniform_int_distribution;

namespace rltk
{

random_number_generator::random_number_generator()
{
	rng.seed(std::random_device()());
}

int random_number_generator::roll_dice(const int& n, const int& d)
{
	int total = 0;
	uniform_int_distribution<mt19937::result_type> dist_dice(1, d);
	for (int i = 0; i < n; ++i)
	{
		total += dist_dice(rng);
	}
	return total;
}

}
