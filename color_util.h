#ifndef CPUSHER_COLOR_UTIL_H
#define CPUSHER_COLOR_UTIL_H

#include "pattern.h"

/*
 * hsv_i
 *
 * This function takes a hue, saturation, and value (https://en.wikipedia.org/wiki/HSL_and_HSV)
 * and mixes them to produce an rgb value. HSV is a more convenient way of manipulating colors
 * than dealing with the red, green, and blue components separately.
 *
 * The inputs on this function are as follows:
 * hue - 0-65535 to go around the color wheel
 * saturation - 0-255
 * value - 0.255
 */
rgb_t hsv_i(uint16_t hue, uint8_t saturation, uint8_t value);

/*
 * mix
 *
 * This function blends two rgb colors. The 'frac' parameter should be between 0 and 1 and
 * determines the ratio: frac=0 returns entirely color1, frac=1 returns color2, in between
 * returns a blend. (frac=0.5 would return a 50/50 mix, and so on.)
 */
rgb_t mix(rgb_t color1, rgb_t color2, double frac);

/*
 * Random number / color generation
 *
 * This provides a way to get "random" values/colors/etc based off of a seed value.
 * Call random_seed() with a number. Then, successive calls to the other random_... functions
 * will return a sequence based off of that seed. A typical way to use this is by calling
 * random_seed(beat_number) at the beginning of a pattern function. Then you'll get a sequence
 * of colors that change randomly with each beat, but are always the same within one beat.
 */
void random_seed(uint32_t seed_value);

/*
 * Generate a random 32-bit integer
 */
uint32_t random_uint();

/*
 * Generate a random double between 0 and 1
 */
double random_normal_double();

/*
 * Generate a random color at full brightness and at least 50% saturation
 */
rgb_t random_bright_color();

#endif
