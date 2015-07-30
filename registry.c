#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "registry.h"

struct pusher_registry registry;
static int registry_pushers_allocated;

static void registry_cond_broadcast();

static int open_listen_socket() {
	struct sockaddr_in addr;
	int reuseaddr_val;

	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		perror("socket");
		exit(1);
	}

	reuseaddr_val = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_val, sizeof reuseaddr_val) < 0) {
		perror("setsockopt SO_REUSEADDR");
		exit(1);
	}

	memset(&addr, 0, sizeof addr);
	addr.sin_port = htons(7331);
	if (bind(fd, (const struct sockaddr *)&addr, sizeof addr) < 0) {
		perror("bind");
		exit(1);
	}

	return fd;
}

static void registry_receive_broadcast(void * buf, int len) {
	struct pusher_broadcast * pb = buf;
	int i;
	(void)len;

	printf("MAC " MAC_FMT ", IP %d.%d.%d.%d, dev id %d, protocol %d, hardware rev %d, "
           "software rev %d, link %d bps, %d pixels/strip, port %d, %d strips/packet\n",
        MAC_FMT_ARGS(pb->mac), (int)pb->ip[0], (int)pb->ip[1], (int)pb->ip[2], (int)pb->ip[3],
        (int)pb->device_type, (int)pb->protocol, (int)pb->hardware_rev, (int)pb->software_rev,
		(int)pb->link_speed, (int)pb->pixels_per_strip, (int)pb->my_port, (int)pb->max_strips_per_packet
	);

	registry_lock();

	/* Is this a device we already know about? */
	for (i = 0; i < registry.num_pushers; i++) {
		if (!memcmp(registry.pushers[i].last_broadcast.mac, pb->mac, sizeof pb->mac)) {
			/* We already know this pusher */
			memcpy(&registry.pushers[i].last_broadcast, pb, sizeof *pb);
			registry.pushers[i].last_seen = time(NULL);
			registry_unlock();
			return;
		}
	}

	printf("Found new pusher " MAC_FMT "\n", MAC_FMT_ARGS(pb->mac));

	if (registry.num_pushers == registry_pushers_allocated) {
		registry_pushers_allocated *= 2;
		registry.pushers = realloc(registry.pushers,
		                           registry_pushers_allocated * sizeof(struct pusher_info));
	}

	i = registry.num_pushers;
	registry.pushers[i].last_seen = time(NULL);
	memcpy(&registry.pushers[i].last_broadcast, pb, sizeof *pb);

	registry.num_pushers++;
	registry_cond_broadcast();
	registry_unlock();
}

void *registry_run(void *unused) {
	int fd = open_listen_socket();
	(void)unused;

	while (1) {
		struct sockaddr_in recv_addr;
		socklen_t recv_addr_len = sizeof recv_addr;
		char buf[1525];
		int ret;

		memset(&recv_addr, 0, sizeof recv_addr);
		ret = recvfrom(fd, buf, sizeof buf, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
		if (ret < 0) {
			perror("recvfrom");
			exit(1);
		}

		memset(buf + ret, 0, (sizeof buf) - ret);
		registry_receive_broadcast(buf, ret);
	}
}

#define PTHR_CHK(x) do { errno = x; if (errno) { perror(#x); exit(1); } } while(0)

static pthread_mutex_t registry_mutex;
static pthread_cond_t registry_cond;

void registry_lock() {
	PTHR_CHK(pthread_mutex_lock(&registry_mutex));
}
void registry_unlock() {
	PTHR_CHK(pthread_mutex_unlock(&registry_mutex));
}
void registry_wait() {
	PTHR_CHK(pthread_cond_wait(&registry_cond, &registry_mutex));
}
static void registry_cond_broadcast() {
	PTHR_CHK(pthread_cond_broadcast(&registry_cond));
}

static void registry_start_thread() {
	pthread_t thr;
	pthread_attr_t attr;

	PTHR_CHK(pthread_attr_init(&attr));
	PTHR_CHK(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));
	PTHR_CHK(pthread_create(&thr, &attr, registry_run, NULL));
	PTHR_CHK(pthread_attr_destroy(&attr));
}

static pthread_once_t registry_once = PTHREAD_ONCE_INIT;

void registry_init() {
	PTHR_CHK(pthread_mutex_init(&registry_mutex, NULL));
	PTHR_CHK(pthread_cond_init(&registry_cond, NULL));
	registry.num_pushers = 0;
	registry_pushers_allocated = 64;
	registry.pushers = malloc(registry_pushers_allocated * sizeof(struct pusher_info));
	pthread_once(&registry_once, registry_start_thread);
}
