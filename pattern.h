#ifndef CPUSHER_PATTERN_H
#define CPUSHER_PATTERN_H

#include <stdint.h>
#include <math.h>

typedef struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} __attribute__((packed)) rgb_t;

struct pattern_info {
    const char *name;
    rgb_t(*func)(int, double);
};

extern struct pattern_info *pattern_arr;
extern int pattern_count;

void register_pattern(const char *name, rgb_t(*pattern)(int, double));

#define PATTERN(x) \
    rgb_t pat__##x(int, double); \
    __attribute__((constructor)) void pat_reg__##x() { register_pattern(#x, pat__##x); } \
    rgb_t pat__##x(int pixel_number, double beat_counter)

/* M_PI is super useful. */
#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#endif /* CPUSHER_PATTERN_H */
