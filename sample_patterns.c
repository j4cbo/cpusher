#include "pattern.h"

#include <stdio.h>

#define BLINK_BRIGHTNESS 255
PATTERN(blink) {
    rgb_t out = {0, 0, 0};
    double phase = fmod(beat_counter, 1.0);
    (void)pixel_number;
    if (phase < .1 && ((int)beat_counter % 12) == (pixel_number % 13)) {
        if (pixel_number % 3 == 0) out.r = BLINK_BRIGHTNESS;
        else if (pixel_number % 3 == 1) out.g = BLINK_BRIGHTNESS;
        else out.b = BLINK_BRIGHTNESS;
    }
    return out;
}

rgb_t hsv(double h, double s, double v) {
    rgb_t out = { 0, 0, 0 };
    double r, g, b, f, p, q, t;
    int i;

    /* If case we got a wacky value of h, wrap it. */
    if (h < 0 || h > 1) {
        h = fmod(h, 1.0);
        if (h < 0) {
            h += 1;
        }
    }

    i = h * 6;
    f = h * 6 - i;
    p = v * (1 - s);
    q = v * (1 - f * s);
    t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }

    out.r = r * 255;
    out.g = g * 255;
    out.b = b * 255;
    return out;
}

#define BRIGHTNESS 255
PATTERN(simple_rainbow) {
    float theta = -(pixel_number * .2) + beat_counter;
    return hsv(theta / (2*M_PI), 1, 1);
}
