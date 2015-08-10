#include "pattern.h"
#include "color_util.h"

#include <stdlib.h>

/* this pattern sucks on its own. might be useful to mix with others though.
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
*/

const rgb_t black = { 0, 0, 0 };

PATTERN(wipe) {
    /* for how much of each beat does the wipe happen? */
    #define WIPE_PORTION 0.3

    /* we want the wipe to pass halfway on the beat, not start on the beat, so adjust the counter */
    double beat_counter_offset = beat_counter + (WIPE_PORTION / 2);

    /* where in the beat are we, and on which integer beat are we? */
    double phase = fmod(beat_counter_offset, 1.0);
    int beat_number = beat_counter_offset;

    /* Pick some "random" colors from the current and next beat number. */
    rgb_t wipe_prev_color, wipe_next_color;
    random_seed(beat_number);
    wipe_prev_color = random_bright_color();
    random_seed(beat_number + 1);
    wipe_next_color = random_bright_color();

    if (phase < WIPE_PORTION) {
        double transition_level = phase / WIPE_PORTION;

        /* Tree positions range from -1 to 1. Normalize this to 0-1. Change our metric on
         * each beat, too. */
        double pos;
        switch (beat_number % 4) {
        case 0: pos = (pixel_x / 2) + 0.5; break;
        case 1: pos = (pixel_y / 2) + 0.5; break;
        case 2: pos = (-pixel_x / 2) + 0.5; break;
        case 3: pos = (-pixel_y / 2) + 0.5; break;
        }

        if (pos > transition_level) {
            return wipe_prev_color;
        } else {
            return wipe_next_color;
        }
    } else {
        return wipe_next_color;
    }
}

PATTERN(spiral) {
    double pos, theta;
    pos = atan2(pixel_x, pixel_y)*5 + sqrt(pixel_x*pixel_x + pixel_y*pixel_y)*10;
    theta = pos + beat_counter;
    return hsv_i(theta / (2*M_PI) * 65536, 255, 255);
}

PATTERN(spinning_rainbow) {
    double pos, theta;
    pos = pixel_x * cos(beat_counter / 4)
        + pixel_y * sin(beat_counter / 4);
    theta = (pos * 3) + beat_counter;
    return hsv_i(theta / (2*M_PI) * 65536, 255, 255);
}

PATTERN(simple_rainbow) {
    double theta = -(pixel_number * .2) + beat_counter;
    return hsv_i(theta / (2*M_PI) * 65536, 255, 255);
}
