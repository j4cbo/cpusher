#include "color_util.h"

/*
 * hsv
 *
 * This function takes a hue, saturation, and value (https://en.wikipedia.org/wiki/HSL_and_HSV)
 * and mixes them to produce an rgb value. HSV is a more convenient way of manipulating colors
 * than dealing with the red, green, and blue components separately.
 */
rgb_t hsv_i(uint16_t hue, uint8_t saturation, uint8_t value) {
    rgb_t out = { 0, 0, 0 };

    // which slice of the color wheel are we in?
    int index = (uint32_t)hue / ((1 << 16) / 6);

    // how far into that slice are we?
    uint32_t hue_frac = ((uint32_t)hue * 6) + 3 - (index << 16);

    uint8_t p = ((uint32_t)value * (uint32_t)(256 - saturation)) >> 8;

    uint32_t zpi;
    if (index % 2 == 0) {
        zpi = (((1 << 16) - hue_frac) * saturation);
    } else {
        zpi = (hue_frac * saturation);
    }

    uint32_t zf = (uint32_t)value * ((1 << 24) - zpi);
    uint8_t z = zf >> 24;

    switch (index % 6) {
        case 0:
            out.r = value;
            out.g = z;
            out.b = p;
        break;
        case 1:
            out.r = z;
            out.g = value;
            out.b = p;
        break;
        case 2:
            out.r = p;
            out.g = value;
            out.b = z;
        break;
        case 3:
            out.r = p;
            out.g = z;
            out.b = value;
        break;
        case 4:
            out.r = z;
            out.g = p;
            out.b = value;
        break;
        case 5:
            out.r = value;
            out.g = p;
            out.b = z;
        break;
    }
    return out;
}

/*
 * mix
 *
 * This function blends two rgb colors. The 'frac' parameter should be between 0 and 1 and
 * determines the ratio: frac=0 returns entirely color1, frac=1 returns color2, in between
 * returns a blend. (frac=0.5 would return a 50/50 mix, and so on.)
 */
rgb_t mix(rgb_t color1, rgb_t color2, double frac) {
    /* Clamp frac to be between 0 and 1 (inclusive), since values outside that range wouldn't
     * make sense. */
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;

    /* Mix each component separately. */
    rgb_t out;
    out.r = (color1.r * (1-frac)) + (color2.r * frac);
    out.g = (color1.g * (1-frac)) + (color2.g * frac);
    out.b = (color1.b * (1-frac)) + (color2.b * frac);
    return out;
}

/*
 * Mechanism for generating a 'random' color. This uses a multiply-with-carry RNG from
 * https://en.wikipedia.org/wiki/Random_number_generation - it's better than rand()/srand()
 * for our purposes.
 */
static int32_t random_w, random_z;

/*
 * Initialize the random number generator
 */
void random_seed(uint32_t seed_value) {
    random_w = 12345 + seed_value;
    random_z = 6789 + seed_value;
}

/*
 * Generate a random 32-bit integer
 */
uint32_t random_uint() {
    random_z = 36969 * (random_z & 65535) + (random_z >> 16);
    random_w = 18000 * (random_w & 65535) + (random_w >> 16);
    return (random_z << 16) + random_w;
}

/*
 * Generate a random double between 0 and 1
 */
double random_normal_double() {
    return (double)random_uint() / UINT32_MAX;
}

/*
 * Generate a random color at full brightness and at least 50% saturation
 */
rgb_t random_bright_color() {
    uint32_t u = random_uint();
    rgb_t out = { 0, 0, 0 };
    #define X1 (u & 0xff)
    #define X2 ((u >> 8) & 0xff)

    switch ((u >> 24) % 6) {
    case 0: out.r = 255; out.g = X1; out.b = X2; break;
    case 1: out.r = 255; out.g = X2; out.b = X1; break;
    case 2: out.r = X1; out.g = 255; out.b = X2; break;
    case 3: out.r = X2; out.g = 255; out.b = X1; break;
    case 4: out.r = X1; out.g = X2; out.b = 255; break;
    case 5: out.r = X2; out.g = X1; out.b = 255; break;
    }
    return out;
}
