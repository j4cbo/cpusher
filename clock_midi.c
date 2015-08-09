#include <alsa/asoundlib.h>
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include <stdio.h>

/* returns the number of nanoseconds each *tick* should be (24 ticks per beat) */
#define NANOSECONDS_FROM_BPM(bpm) (2500000000ULL / (bpm))
#define MIN_TICK_NS     NANOSECONDS_FROM_BPM(250)
#define MAX_TICK_NS     NANOSECONDS_FROM_BPM(40)

static int64_t current_time() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)t.tv_sec * 1000000000 + (int64_t)t.tv_nsec;
}

/*
 * Fastclk rises by 1 per beat.
 *
 * TODO: represent this in a way not subject to fp drift
 */
static pthread_mutex_t fastclk_mutex;
static int64_t last_beat_time = 0;
static double fastclk_nanoseconds_per_beat = NANOSECONDS_FROM_BPM(120);
static double fastclk_at_last_beat = 0;

double beat_clock() {
    pthread_mutex_lock(&fastclk_mutex);
    int64_t now = current_time();
    double time_since_last_beat = now - last_beat_time;
    double fastclk_growth = time_since_last_beat / fastclk_nanoseconds_per_beat;
    double ret = (fastclk_at_last_beat + fastclk_growth) / 24;
    pthread_mutex_unlock(&fastclk_mutex);
    return ret;
}


static void handle_beat() {
    int64_t now = current_time();
    double time_since_last_beat = now - last_beat_time;

    pthread_mutex_lock(&fastclk_mutex);

    if (time_since_last_beat < MIN_TICK_NS || time_since_last_beat > MAX_TICK_NS) {
        // Just ignore totally out-of-range ticks
        last_beat_time = now;
        pthread_mutex_unlock(&fastclk_mutex);
        return;
    }

    printf("expected: %f\tobserved: %f\t",NANOSECONDS_FROM_BPM(fastclk_nanoseconds_per_beat), NANOSECONDS_FROM_BPM(time_since_last_beat));

    double fastclk_growth = time_since_last_beat / fastclk_nanoseconds_per_beat;
    double new_fastclk = fastclk_at_last_beat + fastclk_growth;

    printf("fastclk %f -> now %f\t", fastclk_growth, new_fastclk);

    fastclk_nanoseconds_per_beat = (fastclk_nanoseconds_per_beat * .5)
                                 + (time_since_last_beat * .5);

    double phase = fmod(new_fastclk, 1.0);
    if (phase > 0.5) {
        phase = phase - 1;
    }

    printf("phase off %+f  ", phase);

    double aphase = fabs(phase);
    if (aphase > 0.2) printf("XXXXXXXX\n");
    else if (aphase > 0.1) printf("------\n");
    else if (aphase > 0.05) printf("----\n");
    else if (aphase > 0.01) printf("--\n");
    else if (aphase > .005) printf("-\n");
    else printf(".\n");
    fastclk_nanoseconds_per_beat += (2500000 * phase);

    fastclk_at_last_beat = new_fastclk;
    last_beat_time = now;
    pthread_mutex_unlock(&fastclk_mutex);
}

static void handle_button() {
    printf("PHASE RESET\n");
    pthread_mutex_lock(&fastclk_mutex);
    fastclk_at_last_beat = fmod(fastclk_at_last_beat, 1.0);
    pthread_mutex_unlock(&fastclk_mutex);
}

static char seen_button = 0;

static void handle_byte(char b) {
    /* For now, only handle F8 (beat clock) bytes.
     */
    if (b == 0xf8) {
        handle_beat();
    } else if (b == 0xb0 && seen_button == 0) {
        seen_button = 1;
    } else if (b == 0x4e && seen_button == 1) {
        seen_button = 2;
    } else if (b == 0x7f && seen_button == 2) {
        handle_button();
    } else {
        seen_button = 0;
    }
}

static void read_loop(snd_rawmidi_t *rm) {
    char buf[64];

    while (1) {
        int ret = snd_rawmidi_read(rm, buf, sizeof buf);
        if (ret == 0) {
            continue;
        }

        if (ret < 0) {
            fprintf(stderr, "read error: %s\n", snd_strerror(ret));
            /* just bail - the main loop will try closing and reopening */
            return;
        }

        int i;
        for (i = 0; i < ret; i++) {
            handle_byte(buf[i]);
        }
    }
}

static void attempt_open_device(int card, int device) {
    char name[16];
    snprintf(name, sizeof name, "hw:%d,%d,0", card, device);

    snd_rawmidi_t *rm;
    int ret = snd_rawmidi_open(&rm, NULL, name, 0 /*flags*/);
    if (ret < 0) {
        fprintf(stderr, "read error: %s\n", snd_strerror(ret));
        return;
    }

    snd_rawmidi_read(rm, NULL, 0); /* trigger reading */

    read_loop(rm);
    snd_rawmidi_close(rm);
}

static void open_once() {
    snd_ctl_t *ctl;

    /* Find the first MIDI output device available, and use it */
    int card = -1;
    while (1) {
        int ret = snd_card_next(&card);
        if (ret < 0) {
            fprintf(stderr, "snd_card_next error: %s\n", snd_strerror(ret));
            return;
        }

        if (card < 0) {
            fprintf(stderr, "no suitable cards found\n");
            return;
        }

        char name[16];
        snprintf(name, sizeof name, "hw:%d", card);
        ret = snd_ctl_open(&ctl, name, 0);
        if (ret < 0) {
            fprintf(stderr, "snd_ctl_open error: %s\n", snd_strerror(ret));
            break;
        }

        int device = -1;
        while (1) {
            ret = snd_ctl_rawmidi_next_device(ctl, &device);
            if (ret < 0) {
                fprintf(stderr, "snd_ctl_rwamidi_next_device error: %s\n", snd_strerror(ret));
                return;
            }

            if (device < 0) {
                /* on to the next card (if any) */
                break;
            }

            /* Cool! We found a device! */
            attempt_open_device(card, device);
        }

        snd_ctl_close(ctl);
    }
}

static void *beat_clock_main_loop(void *arg) {
    (void)arg;
    while (1) {
        open_once();
        sleep(1);
    }
    return NULL;
}

#define PTHR_CHK(x) do { errno = x; if (errno) { perror(#x); exit(1); } } while(0)

static void beat_clock_start_thread() {
    pthread_t thr;
    pthread_attr_t attr;

    PTHR_CHK(pthread_attr_init(&attr));
    PTHR_CHK(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));
    PTHR_CHK(pthread_create(&thr, &attr, beat_clock_main_loop, NULL));
    PTHR_CHK(pthread_attr_destroy(&attr));
}

static pthread_once_t beat_clock_init_once = PTHREAD_ONCE_INIT;

void beat_clock_init() {
    PTHR_CHK(pthread_mutex_init(&fastclk_mutex, NULL));
    last_beat_time = current_time();
    pthread_once(&beat_clock_init_once, beat_clock_start_thread);
}
