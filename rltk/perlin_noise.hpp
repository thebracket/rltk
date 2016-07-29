// THIS CLASS IS A TRANSLATION TO C++11 FROM THE REFERENCE
// JAVA IMPLEMENTATION OF THE IMPROVED PERLIN FUNCTION (see http://mrl.nyu.edu/~perlin/noise/)
// THE ORIGINAL JAVA IMPLEMENTATION IS COPYRIGHT 2002 KEN PERLIN

#pragma once

#include <vector>

namespace rltk {

class perlin_noise {
	// The permutation vector
	std::vector<int> p;
public:
	// Initialize with the reference values for the permutation vector
	perlin_noise();
	// Generate a new permutation vector based on the value of seed
	perlin_noise(unsigned int seed);
	// Get a noise value, for 2D images z can have any value
	double noise(double x, double y, double z) const noexcept;
	double noise_octaves(double x, double y, double z, int octaves, double persistence, double frequency) const noexcept;
private:
	double fade(double t) const noexcept;
	double lerp(double t, double a, double b) const noexcept;
	double grad(int hash, double x, double y, double z) const noexcept;
};

}