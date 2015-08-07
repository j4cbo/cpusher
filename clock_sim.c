#include "clock.h"
#include <sys/time.h>
#include <stdio.h>

static struct timeval start_time;

void beat_clock_init() {
    gettimeofday(&start_time, NULL);
}

#define BPM 174

double beat_clock() {
    struct timeval now, diff;
    gettimeofday(&now, NULL);
    timersub(&now, &start_time, &diff);
    double seconds = (double)diff.tv_sec + (double)diff.tv_usec / 1000000.0;
    return seconds * (double)BPM / 60.0;
}
