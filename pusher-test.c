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
#include <time.h>

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

static void send_pattern_to_pusher(int pusher, int pattern, float idx, int *pixel_ptr /*inout*/) {
    const struct pusher_broadcast *pb = &registry.pushers[pusher].last_broadcast;
    char buffer[1536];
    int strip = 0;
    int pixel = *pixel_ptr;
    int computed_spp = ((sizeof buffer) - 4) / (pb->pixels_per_strip * 3 + 1);
    int max_strips_per_packet = pb->max_strips_per_packet;
    struct sockaddr_in dest;
    size_t this_packet_size;
    fill_pusher_address(&dest, pusher);

    if (max_strips_per_packet > computed_spp) {
        max_strips_per_packet = computed_spp;
    }

    buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;

    while (strip < pb->strips_attached) {
        /* Build a packet */
        int strip_in_packet = 0;
        while (strip_in_packet < max_strips_per_packet && strip < pb->strips_attached) {
            int i;
            int buffer_offset = 4 + (pb->pixels_per_strip * 3 + 1) * strip_in_packet;
            buffer[buffer_offset] = strip;
            for (i = 0; i < pb->pixels_per_strip; i++) {
                rgb_t rgb = pattern_arr[pattern].func(pixel++, idx);
                buffer[buffer_offset + 1 + 3*i] = rgb.r;
                buffer[buffer_offset + 2 + 3*i] = rgb.g;
                buffer[buffer_offset + 3 + 3*i] = rgb.b;
            }

            strip++;
            strip_in_packet++;
        }

        this_packet_size = 4 + strip_in_packet * (pb->pixels_per_strip * 3 + 1);
        if (sendto(pusher_send_fd, buffer, this_packet_size, 0, (const struct sockaddr *)&dest, sizeof dest) < 0) {
            perror("sendto");
        }
    }

    *pixel_ptr = pixel;
}

int main() {
    float idx = 0;
    int pixel;
    pusher_send_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (pusher_send_fd < 0) {
        perror("socket");
        exit(1);
    }

    registry_init();
    registry_lock();
    while (!registry.num_pushers) registry_wait();

    printf("Found pusher " MAC_FMT "\n", MAC_FMT_ARGS(registry.pushers[0].last_broadcast.mac));
    while (1) {
        pixel = 0;
        send_pattern_to_pusher(0 /*pusher*/, 1 /*pattern*/, idx, &pixel);

        /* we run at 60 Hz, so there are 3600 frames per minute
         * beat clock is 138 bpm, so there are 3600 / 138 frames per beat
         */
        idx += (138 / 3600.0);

        usleep(16667);
    }
}
