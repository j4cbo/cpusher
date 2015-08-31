#ifndef CPUSHER_PATTERN_H
#define CPUSHER_PATTERN_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>

/*
 * rgb_t
 *
 * This struct represents the color sent to a single pixel. See color_util.h for some
 * helper functions to create and manipulate colors.
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

/*
 * This defines the parameters that each pattern receives. Stay with me, it's not that bad :)
 *
 * Each pattern is a function that is called once per pixel per frame, to calculate the color
 * that each pixel should have. It receives these inputs:
 *   pixel_x      - the X axis of the location of this pixel
 *   pixel_y      - the Y axis of the location of this pixel
 *   pixel_number - the sequential position of this pixel on its PixelPusher
 *   beat_counter - a double that starts at 0 and counts up continuously, 1 per beat
 *
 * The __attribute__((unused)) flags make the compiler not warn if a pattern doesn't use
 * that variable. (A pattern will likely use either x and y, or pixel_number, but not
 * necessarily both)
 */
#define PATTERN_PARAMETERS __attribute__((unused)) double pixel_x, \
                           __attribute__((unused)) double pixel_y, \
                           __attribute__((unused)) int pixel_number, \
                           double beat_counter

struct pattern_info {
    const char *name;
    rgb_t(*func)(PATTERN_PARAMETERS);
};

extern struct pattern_info *pattern_arr;
extern int pattern_count;

void register_pattern(const char *name, rgb_t(*pattern)(PATTERN_PARAMETERS));

#define PATTERN(x) \
    rgb_t pat__##x(PATTERN_PARAMETERS); \
    __attribute__((constructor)) void pat_reg__##x() { register_pattern(#x, pat__##x); } \
    rgb_t pat__##x(PATTERN_PARAMETERS)

/* M_PI is super useful. */
#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

struct xy {
    double x;
    double y;
};

struct pusher_config {
    uint32_t pusher_id;
    size_t valid_pixels;
    struct xy pixel_locations[1920];
};

extern const struct pusher_config pushers[];
extern const size_t pusher_config_count;
const struct pusher_config * pusher_config_for(uint32_t pusher_id);

#endif /* CPUSHER_PATTERN_H */
