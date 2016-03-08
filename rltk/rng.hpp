#pragma once

#include <random>

namespace rltk
{

class random_number_generator
{
public:
	random_number_generator();

	int roll_dice(const int &n, const int &d);
private:
	std::mt19937 rng;
};

}