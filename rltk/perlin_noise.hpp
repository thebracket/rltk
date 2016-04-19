#pragma once

namespace rltk {

class perlin_noise
{
public:

	// Constructor
	perlin_noise();
	perlin_noise(double _persistence, double _frequency, double _amplitude,
			int _octaves, int _randomseed);

	// Get Height
	double get_height(double x, double y) const;

	// Get
	double persistence() const
	{
		return persistence_d;
	}
	double frequency() const
	{
		return frequency_d;
	}
	double amplitude() const
	{
		return amplitude_d;
	}
	int octaves() const
	{
		return octaves_i;
	}
	int random_seed() const
	{
		return random_seed_i;
	}

	// Set
	void set(double _persistence, double _frequency, double _amplitude,
			int _octaves, int _randomseed);

	void set_persistence(double _persistence)
	{
		persistence_d = _persistence;
	}
	void set_frequency(double _frequency)
	{
		frequency_d = _frequency;
	}
	void set_amplitude(double _amplitude)
	{
		amplitude_d = _amplitude;
	}
	void set_octaves(int _octaves)
	{
		octaves_i = _octaves;
	}
	void set_random_seed(int _randomseed)
	{
		random_seed_i = _randomseed;
	}

private:

	double total(double i, double j) const;
	double get_value(double x, double y) const;
	double interpolate(double x, double y, double a) const;
	double noise(int x, int y) const;

	double persistence_d, frequency_d, amplitude_d;
	int octaves_i, random_seed_i;
};

}