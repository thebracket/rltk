#include "color_t.hpp"
#include <cmath>

using std::min;
using std::max;
using std::fmod;

namespace rltk {

// Credit: https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
std::tuple<float, float, float> color_to_hsv(const color_t &col) {
	float fR = (col.r / 255.0);
	float fG = (col.g / 255.0);
	float fB = (col.b / 255.0);

	float fH=0.0, fS=0.0, fV=0.0;

	float fCMax = max(max(fR, fG), fB);
	float fCMin = min(min(fR, fG), fB);
	float fDelta = fCMax - fCMin;

	if(fDelta > 0) {
		if(fCMax == fR) {
	  		fH = 60 * (fmod(((fG - fB) / fDelta), 6));
		} else if(fCMax == fG) {
	  		fH = 60 * (((fB - fR) / fDelta) + 2);
		} else if(fCMax == fB) {
	  		fH = 60 * (((fR - fG) / fDelta) + 4);
		}

		if(fCMax > 0) {
	  		fS = fDelta / fCMax;
		} else {
			fS = 0;
		}

		fV = fCMax;
	} else {
		fH = 0;
		fS = 0;
		fV = fCMax;
	}

	if(fH < 0) {
		fH = 360 + fH;
	}

	return std::make_tuple(fH, fS, fV);
}

// Credit: https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
std::tuple<uint8_t, uint8_t, uint8_t> color_from_hsv(const float hue, const float saturation, const float value) {
	float fH = hue;
	float fS = saturation;
	float fV = value;

	float fR, fG, fB;

	float fC = fV * fS; // Chroma
	float fHPrime = fmod(fH / 60.0, 6);
	float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
	float fM = fV - fC;

	if(0 <= fHPrime && fHPrime < 1) {
		fR = fC;
		fG = fX;
		fB = 0;
	} else if(1 <= fHPrime && fHPrime < 2) {
		fR = fX;
		fG = fC;
		fB = 0;
	} else if(2 <= fHPrime && fHPrime < 3) {
		fR = 0;
		fG = fC;
		fB = fX;
	} else if(3 <= fHPrime && fHPrime < 4) {
		fR = 0;
		fG = fX;
		fB = fC;
	} else if(4 <= fHPrime && fHPrime < 5) {
		fR = fX;
		fG = 0;
		fB = fC;
	} else if(5 <= fHPrime && fHPrime < 6) {
		fR = fC;
		fG = 0;
		fB = fX;
	} else {
		fR = 0;
		fG = 0;
		fB = 0;
	}

	fR += fM;
	fG += fM;
	fB += fM;

	return std::make_tuple<uint8_t, uint8_t, uint8_t>(static_cast<uint8_t>(fR*255.0), static_cast<uint8_t>(fG*255.0), static_cast<uint8_t>(fB*255.0));
}

/*
 * Calculates the luminance of a color, and converts it to grey-scale.
 */
color_t greyscale(const color_t &col)
{
	unsigned char red = col.r;
	unsigned char green = col.g;
	unsigned char blue = col.b;

	float RED = red / 255.0F;
	float GREEN = green / 255.0F;
	float BLUE = blue / 255.0F;
	float luminance = 0.299 * RED + 0.587 * GREEN + 0.114 * BLUE;

	red = (luminance * 255.0F);
	green = (luminance * 255.0F);
	blue = (luminance * 255.0F);

	return color_t(red, green, blue);
}

/*
 * Darkens a color by the specified amount.
 */
color_t darken(const int &amount, const color_t &col)
{
	unsigned char red = col.r;
	unsigned char green = col.g;
	unsigned char blue = col.b;

	if (red > amount)
	{
		red -= amount;
	}
	else
	{
		red = 0;
	}
	if (green > amount)
	{
		green -= amount;
	}
	else
	{
		green = 0;
	}
	if (blue > amount)
	{
		blue -= amount;
	}
	else
	{
		blue = 0;
	}

	return color_t(red, green, blue);
}

/* Applies colored lighting effect; colors that don't exist remain dark. Lights are from 0.0 to 1.0. */
color_t apply_colored_light(const color_t &col, const std::tuple<float,float,float> &light) {
	unsigned char red = col.r;
	unsigned char green = col.g;
	unsigned char blue = col.b;

	float RED = red / 255.0F;
	float GREEN = green / 255.0F;
	float BLUE = blue / 255.0F;

	RED *= std::get<0>(light);
	GREEN *= std::get<1>(light);
	BLUE *= std::get<2>(light);

	if (RED > 1.0) RED = 1.0;
	if (RED < 0.0) RED = 0.0;
	if (GREEN > 1.0) GREEN = 1.0;
	if (GREEN < 0.0) GREEN = 0.0;
	if (BLUE > 1.0) BLUE = 1.0;
	if (BLUE < 0.0) BLUE = 0.0;

	red = (RED * 255.0F);
	green = (GREEN * 255.0F);
	blue = (BLUE * 255.0F);

	return color_t(red, green, blue);
}

}