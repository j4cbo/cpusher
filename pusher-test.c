#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "registry.h"
#include "pattern.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include "clock.h"

int pusher_send_fd;

#define BRIGHTNESS 127

static void fill_pusher_address(struct sockaddr_in * dest, int i) {
    memset(dest, 0, sizeof *dest);
    dest->sin_family = AF_INET;
    dest->sin_port = htons(registry.pushers[i].last_broadcast.my_port);
    dest->sin_addr.s_addr = (registry.pushers[i].last_broadcast.ip[3] << 24)
                 | (registry.pushers[i].last_broadcast.ip[2] << 16)
                 | (registry.pushers[i].last_broadcast.ip[1] << 8)
                 | registry.pushers[i].last_broadcast.ip[0];
}

static int send_pattern_to_pusher(int pusher, int pattern, float idx) {
    const struct pusher_broadcast *pb = &registry.pushers[pusher].last_broadcast;
    char buffer[1536];
    int strip = 0;
    size_t pixel = 0;
    int pixels_calculated = 0;
    int computed_spp = ((sizeof buffer) - 4) / (pb->pixels_per_strip * 3 + 1);
    int max_strips_per_packet = pb->max_strips_per_packet;
    struct sockaddr_in dest;
    size_t this_packet_size;
    fill_pusher_address(&dest, pusher);

    if (max_strips_per_packet > computed_spp) {
        max_strips_per_packet = computed_spp;
    }

    buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;

    const struct pusher_config *cfg = pusher_config_for(registry.pushers[pusher].id);
    if (!cfg) {
        return 0;
    }

    while (strip < pb->strips_attached) {
        /* Build a packet */
        int strip_in_packet = 0;
        while (strip_in_packet < max_strips_per_packet && strip < pb->strips_attached) {
            int i;
            int buffer_offset = 4 + (pb->pixels_per_strip * 3 + 1) * strip_in_packet;
            buffer[buffer_offset] = strip;
            for (i = 0; i < pb->pixels_per_strip; i++) {
                if (pixel >= cfg->valid_pixels) {
                    buffer[buffer_offset + 1 + 3*i] = 0;
                    buffer[buffer_offset + 2 + 3*i] = 0;
                    buffer[buffer_offset + 3 + 3*i] = 0;
                } else {
                    rgb_t rgb = pattern_arr[pattern].func(
                        cfg->pixel_locations[pixel].x,
                        cfg->pixel_locations[pixel].y,
                        pixel,
                        idx
                    );
                    buffer[buffer_offset + 1 + 3*i] = rgb.r;
                    buffer[buffer_offset + 2 + 3*i] = rgb.g;
                    buffer[buffer_offset + 3 + 3*i] = rgb.b;
                    pixel++;
                    pixels_calculated++;
                }
            }

            strip++;
            strip_in_packet++;
        }

        this_packet_size = 4 + strip_in_packet * (pb->pixels_per_strip * 3 + 1);
        if (sendto(pusher_send_fd, buffer, this_packet_size, 0, (const struct sockaddr *)&dest, sizeof dest) < 0) {
            perror("sendto");
        }
    }

    return pixels_calculated;
}

#define FPS 60

int main() {
    int pusher;

    pusher_send_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (pusher_send_fd < 0) {
        perror("socket");
        exit(1);
    }

    beat_clock_init();

    struct timeval last_frame;
    struct timeval frame_interval = { 0, 1000000 / FPS };
    gettimeofday(&last_frame, NULL);

    uint64_t sleep_total_time = 0;
    int sleep_count = 0;
    int frame_counter = 0;

    registry_init();
    registry_lock();

    while (1) {
        double idx = beat_clock();
        int pixels_pushed = 0;
        for (pusher = 0; pusher < registry.num_pushers; pusher++) {
            pixels_pushed += send_pattern_to_pusher(pusher, 1 /*pattern*/, idx);
        }

        registry_unlock();

        if (++frame_counter == FPS) {
            frame_counter = 0;
            if (sleep_count) {
                printf("%d pixels; avg sleep: %d\n", pixels_pushed, (int)(sleep_total_time / sleep_count));
            }

            for (pusher = 0; pusher < registry.num_pushers; pusher++) {
                if (!pusher_config_for(registry.pushers[pusher].id)) {
                    printf("\n/!\\ UNKNOWN PUSHER: %06x\n\n", registry.pushers[pusher].id);
                }
            }

            sleep_count = 0;
            sleep_total_time = 0;
        }

        struct timeval now, next_frame, diff;
        gettimeofday(&now, NULL);
        timeradd(&last_frame, &frame_interval, &next_frame);
        if (timercmp(&now, &next_frame, <)) {
            timersub(&next_frame, &now, &diff);
            uint64_t usec = (uint64_t)diff.tv_sec * 1000000 + (uint64_t)diff.tv_usec;
            usleep(usec);
            sleep_total_time += usec;
            sleep_count++;
        } else {
            timersub(&now, &next_frame, &diff);
            uint64_t usec = (uint64_t)diff.tv_sec * 1000000 + (uint64_t)diff.tv_usec;
            printf("dropped frame - %lld us\n", (long long)usec);
            memcpy(&next_frame, &now, sizeof now);
        }

        memcpy(&last_frame, &next_frame, sizeof next_frame);

        registry_lock();
    }
}
