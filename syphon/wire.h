#ifndef CPUSHER_WIRE_H
#define CPUSHER_WIRE_H

#include <stdint.h>

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_FMT_ARGS(x) (int)x[0], (int)x[1], (int)x[2], (int)x[3], (int)x[4], (int)x[5]

struct pusher_broadcast {
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t device_type;
    uint8_t protocol;
    uint16_t vid;
    uint16_t pid;
    uint16_t hardware_rev;
    uint16_t software_rev;
    uint32_t link_speed;
    uint8_t  strips_attached;
    uint8_t  max_strips_per_packet;
    uint16_t pixels_per_strip;
    uint32_t update_period;
    uint32_t power_total;
    uint32_t delta_sequence;
    int32_t controller_ordinal;
    int32_t group_ordinal;
    uint16_t artnet_universe;
    uint16_t artnet_channel;
    uint16_t my_port;
    uint16_t padding1;
    uint8_t strip_flags[8];
    uint16_t padding2;
    uint32_t pusher_flags;
    uint32_t segments;
    uint32_t power_domain;
    uint8_t last_driven_ip[4];
    uint16_t last_driven_port;
} __attribute__((packed));

#endif /* CPUSHER_WIRE_H */
