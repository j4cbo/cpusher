#ifndef CPUSHER_COLOR_UTIL_H
#define CPUSHER_COLOR_UTIL_H

#include "pattern.h"

/*
 * hsv
 *
 * This function takes a hue, saturation, and value (https://en.wikipedia.org/wiki/HSL_and_HSV)
 * and mixes them to produce an rgb value. HSV is a more convenient way of manipulating colors
 * than dealing with the red, green, and blue components separately.
 */
rgb_t hsv(double hue, double saturation, double value);

/*
 * mix
 *
 * This function blends two rgb colors. The 'frac' parameter should be between 0 and 1 and
 * determines the ratio: frac=0 returns entirely color1, frac=1 returns color2, in between
 * returns a blend. (frac=0.5 would return a 50/50 mix, and so on.)
 */
rgb_t mix(rgb_t color1, rgb_t color2, double frac);

#endif
