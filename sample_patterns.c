#include "pattern.h"

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

rgb_t mix(rgb_t c1, rgb_t c2, double frac) {
    rgb_t out;
    if (frac < 0) frac = 0;
    if (frac > 1) frac = 1;
    out.r = (c1.r * (1-frac)) + (c2.r * frac);
    out.g = (c1.g * (1-frac)) + (c2.g * frac);
    out.b = (c1.b * (1-frac)) + (c2.b * frac);
    return out;
}

/*
 * Mechanism for generating a 'random' color. This uses a multiply-with-carry RNG from
 * https://en.wikipedia.org/wiki/Random_number_generation - more random than rand()/srand().
 */
static int32_t random_w, random_z;
static uint32_t random_uint() {
    random_z = 36969 * (random_z & 65535) + (random_z >> 16);
    random_w = 18000 * (random_w & 65535) + (random_w >> 16);
    return (random_z << 16) + random_w;
}
static void random_seed(uint32_t seed_value) {
    random_w = 12345;
    random_z = 6789 + seed_value;
}

/*
 * Retunrs a random color at full brightness and at least 50% saturation.
 */
rgb_t random_color() {
    return hsv((double)random_uint() / UINT32_MAX,
               0.5 + ((double)random_uint() / UINT32_MAX / 2),
               1);
}

const rgb_t black = { 0, 0, 0 };

PATTERN(wipe) {
    const struct pusher_config *cfg;

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
    wipe_prev_color = random_color();
    random_seed(beat_number + 1);
    wipe_next_color = random_color();

    cfg = pusher_config_for(pusher_id);
    if (!cfg) {
        return black;
    }

    if (phase < WIPE_PORTION) {
        double transition_level = phase / WIPE_PORTION;

        /* Tree positions range from -1 to 1. Normalize this to 0-1. Change our metric on
         * each beat, too. */
        double pos;
        switch (beat_number % 4) {
        case 0: pos = (cfg->pixel_locations[pixel_number].x / 2) + 0.5; break;
        case 1: pos = (cfg->pixel_locations[pixel_number].y / 2) + 0.5; break;
        case 2: pos = (-cfg->pixel_locations[pixel_number].x / 2) + 0.5; break;
        case 3: pos = (-cfg->pixel_locations[pixel_number].y / 2) + 0.5; break;
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

PATTERN(spinning_rainbow) {
    const struct pusher_config *cfg = pusher_config_for(pusher_id);
    double pos, theta;
    if (!cfg) {
        return black;
    }
    pos = cfg->pixel_locations[pixel_number].x * cos(beat_counter / 4)
        + cfg->pixel_locations[pixel_number].y * sin(beat_counter / 4);
    theta = (pos * 3) + beat_counter;
    return hsv(theta / (2*M_PI), 1, 1);
}

PATTERN(simple_rainbow) {
    float theta = -(pixel_number * .2) + beat_counter;
    return hsv(theta / (2*M_PI), 1, 1);
}
