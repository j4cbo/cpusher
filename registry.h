#ifndef CPUSHER_REGISTRY_H
#define CPUSHER_REGISTRY_H

#include "wire.h"
#include <sys/types.h>

/*
 * Pixelpusher Registry
 *
 * The registry is the set of all PixelPushers that have ever been seen by the lib. Pushers
 * are never forgotten by the registry.
 *
 * The registry is constantly updated by a background thread that listens for broadcasts,
 * adds new pushers to the registry, and updates existing pushers.
 */
struct pusher_info {
    struct pusher_broadcast last_broadcast;
    time_t last_seen;
};

struct pusher_registry {
    struct pusher_info *pushers;
    int num_pushers;
};

extern struct pusher_registry registry;

/*
 * Initialize the registry. Call this before doing anything else.
 */
void registry_init();

/*
 * Lock and unlock the registry. All access to the 'registry' global must be done
 * with the registry lock held.
 */
void registry_lock();
void registry_unlock();

/*
 * Wait until something in the registry changes (a broadcast packet is received).
 * This must be called with the registry lock held.
 */
void registry_wait();

#endif /* CPUSHER_REGISTRY_H */
