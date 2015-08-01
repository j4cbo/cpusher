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

static void send_to_pusher(void * buf, size_t n, int i) {
    struct sockaddr_in dest;
    fill_pusher_address(&dest, i);
    if (sendto(pusher_send_fd, buf, n, 0, (const struct sockaddr *)&dest, sizeof dest) < 0) {
        perror("sendto");
        exit(1);
    }
}

int main() {
    uint8_t buf[((240*3) + 1) * 2 + 4];
    int i;
    float idx = 0;
    pusher_send_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (pusher_send_fd < 0) {
        perror("socket");
        exit(1);
    }

    registry_init();
    registry_lock();
    while (!registry.num_pushers) registry_wait();

    printf("Found pusher " MAC_FMT "\n", MAC_FMT_ARGS(registry.pushers[0].last_broadcast.mac));

    memset(buf, 0, sizeof buf);
    buf[4] = 0;
    buf[725] = 1;

    while (1) {
        for (i = 0; i < 240; i++) {
            rgb_t pixel = pattern_arr[1].func(i, idx);
            buf[5 + (3*i) + 0] = pixel.r;
            buf[5 + (3*i) + 1] = pixel.g;
            buf[5 + (3*i) + 2] = pixel.b;
        }

        /* we run at 60 Hz, so there are 3600 frames per minute
         * beat clock is 138 bpm, so there are 3600 / 138 frames per beat
         */
        idx += (138 / 3600.0);

        send_to_pusher(buf, sizeof buf, 0);

        usleep(16667);
    }
}
