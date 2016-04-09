#include "rng.hpp"

#include <iostream>
#include <functional>

using std::mt19937;
using std::uniform_int_distribution;

namespace rltk
{

random_number_generator::random_number_generator()
{
	rng.seed(std::random_device()());
}

random_number_generator::random_number_generator(const uint64_t seed)
{
	rng.seed(seed);
}

random_number_generator::random_number_generator(const std::string seed) {
	std::hash<std::string> hash_func;
	rng.seed(hash_func(seed));
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
