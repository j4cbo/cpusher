#include "color_util.h"

/*
 * hsv
 *
 * This function takes a hue, saturation, and value (https://en.wikipedia.org/wiki/HSL_and_HSV)
 * and mixes them to produce an rgb value. HSV is a more convenient way of manipulating colors
 * than dealing with the red, green, and blue components separately.
 */
rgb_t hsv(double hue, double saturation, double value) {
    rgb_t out = { 0, 0, 0 };
    double r, g, b;

    /* If case we got a wacky value of h, wrap it. */
    if (hue < 0 || hue > 1) {
        hue = fmod(hue, 1.0);
        if (hue < 0) {
            hue += 1;
        }
    }

    // which slice of the color wheel are we in?
    int index = hue * 6;

    // how far into that slice are we?
    double hue_frac = (hue * 6) - index;

    double p = value * (1 - saturation);
    double q = value * (1 - hue_frac * saturation);
    double t = value * (1 - (1 - hue_frac) * saturation);

    switch (index % 6) {
        case 0: r = value; g = t; b = p;        break;
        case 1: r = q;     g = value; b = p;     break;
        case 2: r = p;     g = value; b = t; break;
        case 3: r = p;     g = q;     b = value; break;
        case 4: r = t;     g = p;     b = value; break;
        case 5: r = value; g = p;     b = q; break;
    }

    out.r = r * 255;
    out.g = g * 255;
    out.b = b * 255;
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
