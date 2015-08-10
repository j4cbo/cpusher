#include "pattern.h"
#include <stdio.h>
#include <stdlib.h>

struct pattern_info *pattern_arr = NULL;
int pattern_count = 0;
static int pattern_max = 0;

void register_pattern(const char *name, rgb_t(*pattern)(double, double, int, double)) {
    /* grow the array if need be */
    if (pattern_count == pattern_max) {
        pattern_max *= 2;
        if (!pattern_max) {
            pattern_max = 32;
        }
        pattern_arr = realloc(pattern_arr, pattern_max * sizeof(struct pattern_info));
    }

    pattern_arr[pattern_count].name = name;
    pattern_arr[pattern_count].func = pattern;
    pattern_count++;
}

const struct pusher_config * pusher_config_for(uint32_t pusher_id) {
    size_t i = 0;
    for (i = 0; i < pusher_config_count; i++) {
        if (pushers[i].pusher_id == pusher_id) {
            return &pushers[i];
        }
    }
    return NULL;
}
