#pragma once

#include "color_t.hpp"

// Credit for RGB value assistance: http://roguecentral.org/doryen/data/libtcod/doc/1.5.1/html2/color.html?c=false&cpp=false&cs=false&py=false&lua=false

namespace rltk {

namespace colors {

const color_t BLACK(0,0,0);
const color_t WHITE(255,255,255);

const color_t DESATURATED_RED(128,64,64);
const color_t LIGHTEST_RED(255,191,191);
const color_t LIGHTER_RED(255,166,166);
const color_t LIGHT_RED(255,115,115);
const color_t RED(255,0,0);
const color_t DARK_RED(191,0,0);
const color_t DARKER_RED(128,0,0);
const color_t DARKEST_RED(64,0,0);
const color_t FLAME(255,63,0);
const color_t ORANGE(255,127,0);
const color_t AMBER(255,191,0);
const color_t YELLOW(255,255,0);
const color_t LIME(191,255,0);
const color_t CHARTREUSE(127,255,0);
const color_t DESATURATED_GREEN(64,128,64);
const color_t LIGHTEST_GREEN(191,255,191);
const color_t LIGHTER_GREEN(166,255,166);
const color_t LIGHT_GREEN(115,255,115);
const color_t GREEN(0,255,0);
const color_t DARK_GREEN(0,191,0);
const color_t DARKER_GREEN(0,128,0);
const color_t DARKEST_GREEN(0,64,0);
const color_t SEA(0,255,127);
const color_t TURQUOISE(0,255,191);
const color_t CYAN(0,255,255);
const color_t SKY(0,191,255);
const color_t AZURE(0,127,255);
const color_t BLUE(0,0,255);
const color_t HAN(63,0,255);
const color_t VIOLET(127,0,255);
const color_t PURPLE(191,0,255);
const color_t FUCHSIA(255,0,191);
const color_t MAGENTA(255,0,255);
const color_t PINK(255,0,127);
const color_t CRIMSON(255,0,63);

const color_t BRASS(191,151,96);
const color_t COPPER(200,117,51);
const color_t GOLD(229,191,0);
const color_t SILVER(203,203,203);

const color_t CELADON(172,255,171);
const color_t PEACH(255,159,127);

const color_t LIGHTEST_GREY(223,223,223);
const color_t LIGHTER_GREY(191,191,191);
const color_t LIGHT_GREY(159,159,159);
const color_t GREY(127,127,127);
const color_t DARK_GREY(95,95,95);
const color_t DARKER_GREY(63,63,63);
const color_t DARKEST_GREY(31,31,31);

const color_t LIGHTEST_SEPIA(222,211,195);
const color_t LIGHTER_SEPIA(191,171,143);
const color_t LIGHT_SEPIA(158,134,100);
const color_t SEPIA(127,101,63);
const color_t DARK_SEPIA(94,75,47);
const color_t DARKER_SEPIA(63,50,31);
const color_t DARKEST_SEPIA(31,24,15);

}
}
