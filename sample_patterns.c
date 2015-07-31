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

#define BRIGHTNESS 255
PATTERN(simple_rainbow) {
    float theta = -(pixel_number * .2) + beat_counter;
    rgb_t out = {0, 0, 0};
    out.r = ((sin(theta) + 1) / 2) * BRIGHTNESS;
    out.g = ((sin(theta + M_PI*2.0/3.0) + 1) / 2) * BRIGHTNESS;
    out.b = ((sin(theta + M_PI*4.0/3.0) + 1) / 2) * BRIGHTNESS;
    return out;
}
