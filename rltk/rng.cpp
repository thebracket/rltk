#include "rng.hpp"

#include <iostream>
#include <functional>

using std::mt19937;
using std::uniform_int_distribution;

namespace rltk
{

random_number_generator::random_number_generator()
{
	initial_seed = static_cast<uint64_t>(std::random_device()());
	rng.seed(static_cast<unsigned int>(initial_seed));
}

random_number_generator::random_number_generator(const uint64_t seed)
{
	initial_seed = seed;
	rng.seed(static_cast<unsigned int>(seed));
}

random_number_generator::random_number_generator(const std::string seed) {
	std::hash<std::string> hash_func;
	initial_seed = hash_func(seed);
	rng.seed(static_cast<unsigned int>(initial_seed));
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
