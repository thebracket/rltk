#pragma once

/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Random number generator class.
 */

#include <string>

namespace rltk
{

class random_number_generator
{
public:
	random_number_generator();
	random_number_generator(const int seed);
	random_number_generator(const std::string seed);

	int roll_dice(const int &n, const int &d);
	int initial_seed;
private:
    int fastrand();
    int g_seed;
};

}