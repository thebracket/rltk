#pragma once

/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Color functions
 */

#include <tuple>
#include <SFML/Graphics.hpp>
#include <cereal/cereal.hpp>

namespace rltk {

/* Converts HSV into an RGB tuple */
extern std::tuple<uint8_t, uint8_t, uint8_t> color_from_hsv(const float hue, const float saturation, const float value);

struct color_t {
	/* Default empty constructor */
	color_t() {}

	/* Convenience constructor from red/green/blue; accepts ints and casts them */
	color_t(const int R, const int G, const int B) : r(static_cast<uint8_t>(R)), g(static_cast<uint8_t>(G)), b(static_cast<uint8_t>(B)) {}

	/* Construct from red/green/blue, in the range of 0-255 per component. */
	color_t(const uint8_t R, const uint8_t G, const uint8_t B) : r(R), g(G), b(B) {}

	/* Construct from HSV, in the range of 0-1.0 as floats. */
	color_t(const float hue, const float saturation, const float value) {
		std::tie(r,g,b) = color_from_hsv(hue, saturation, value);
	}

	/* Construct from an RGB tuple */
	color_t(const std::tuple<uint8_t, uint8_t, uint8_t> &c) { std::tie(r,g,b) = c; }

	/* You can add colors together, for example as a quick blend/lighten */
	color_t operator+(const color_t &other) {
		int red = r + other.r;
		int green = g + other.g;
		int blue = b + other.b;
		if (red > 255) red = 255;
		if (green > 255) green = 255;
		if (blue > 255) blue = 255;
		return color_t(red, green, blue);
	}

	/* You can subtract colors */
	color_t operator-(const color_t &other) {
		int red = r - other.r;
		int green = g - other.g;
		int blue = b - other.b;
		if (red < 0) red = 0;
		if (green < 0) green = 0;
		if (blue < 0) blue = 0;
		return color_t(red, green, blue);
	}

	/* You can multiply colors */
	color_t operator*(const color_t &other) {
		int red = r * other.r;
		int green = g * other.g;
		int blue = b * other.b;
		if (red < 0) red = 0;
		if (green < 0) green = 0;
		if (blue < 0) blue = 0;
		if (red > 255) red = 255;
		if (green > 255) green = 255;
		if (blue > 255) blue = 255;
		return color_t(red, green, blue);
	}

	/* You can compare colors */
	bool operator==(const color_t &other) const {
		if (other.r == r && other.g == g && other.b == b) {
			return true;
		} else {
			return false;
		}
	}

	/* RGB storage */
	uint8_t r,g,b;

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive( r, g, b ); // serialize things by passing them to the archive
	}
};

/* Converts a color_t to an SFML color */
inline sf::Color color_to_sfml(const color_t &col) { return sf::Color(col.r, col.g, col.b); }

/* Converts a color_t to an RGB tuple */
inline std::tuple<uint8_t, uint8_t, uint8_t> color_to_rgb(const color_t &col) { return std::make_tuple(col.r, col.g, col.b); }

/* Converts a color_t to an HSV tuple */
extern std::tuple<float, float, float> color_to_hsv(const color_t &col);

/* Calculates the luminance of a color, and converts it to grey-scale. */
extern color_t greyscale(const color_t &col);

/* Darkens a color by the specified amount. */
color_t darken(const int &amount, const color_t &col);

/* Applies colored lighting effect; colors that don't exist remain dark. Lights are from 0.0 to 1.0. */
color_t apply_colored_light(const color_t &col, const std::tuple<float,float,float> &light);

/* Calculates an intermediate color on a linear RGB color ramp. Amount is from 0 to 1 */
extern color_t lerp(const color_t &first, const color_t &second, float amount);

}