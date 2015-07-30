#include <unistd.h>
#include <stdio.h>
#include "registry.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

static void fill_pusher_address(struct sockaddr_in * dest, int i) {
    memset(dest, 0, sizeof *dest);
    dest->sin_family = AF_INET;
    dest->sin_port = htons(registry.pushers[i].last_broadcast.my_port);
    dest->sin_addr.s_addr = (registry.pushers[i].last_broadcast.ip[3] << 24)
                 | (registry.pushers[i].last_broadcast.ip[2] << 16)
                 | (registry.pushers[i].last_broadcast.ip[1] << 8)
                 | registry.pushers[i].last_broadcast.ip[0];
}


int main() {
    struct sockaddr_in dest;
    uint8_t buf[2000];
    int i;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    registry_init();
    registry_lock();
    while (!registry.num_pushers) registry_wait();

    printf("Found pusher " MAC_FMT "\n", MAC_FMT_ARGS(registry.pushers[0].last_broadcast.mac));
    fill_pusher_address(&dest, 0);

    memset(buf, 0, sizeof buf);
    for (i = 0; i < 24; i++) {
        buf[5 + (9*i)] = 50;
        buf[5 + (9*i) + 4] = 50;
        buf[5 + (9*i) + 8] = 50;
    }
    printf("sending to %s\n", inet_ntoa(dest.sin_addr));
    if (sendto(fd, buf, 846, 0, (const struct sockaddr *)&dest, sizeof dest) < 0) {
        perror("sendto");
        exit(1);
    }
}
