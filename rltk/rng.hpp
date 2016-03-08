#pragma once

/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the LGPL - see LICENSE file.
 *
 * Random number generator class.
 */

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