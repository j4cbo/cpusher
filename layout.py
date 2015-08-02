from math import sin, cos, pi
import itertools

def cluster6(x, y):
    led_distance = .015
    for rot in range(6):
        for dist in (0, 2, 3, 1):
            yield(x + cos(rot * pi * 2 / 6) * led_distance * (dist + 1.5),
                  y + sin(rot * pi * 2 / 6) * led_distance * (dist + 1.5))

def cluster8(x, y):
    led_distance = .02
    for rot in range(8):
        for dist in (0, 2, 1):
            yield(x + cos(rot * pi * 2 / 8) * led_distance * (dist + 1.5),
                  y + sin(rot * pi * 2 / 8) * led_distance * (dist + 1.5))

def densecluster8(x, y):
    led_distance = .003
    for rot in range(8):
        for dist in range(40):
            yield(x + cos(rot * pi * 2 / 8) * led_distance * (dist + 10),
                  y + sin(rot * pi * 2 / 8) * led_distance * (dist + 10))

config = {
    0x12345: [
        cluster6(0.35, 0.45),
        cluster6(0.25, 0.15),
        cluster6(0.1, 0.6),
        cluster6(0.45, 0.3),
        cluster8(0.8, 0.4),
        cluster8(0.75, 0.8),
        cluster8(0.4, 0.65),
        cluster8(0.6, 0.1),
    ],
    0xcde456: [
        cluster6(-0.45, 0.35),
        cluster6(-0.15, 0.25),
        cluster6(-0.6, 0.1),
        cluster6(-0.3, 0.55),
        cluster8(-0.4, 0.8),
        cluster8(-0.8, 0.75),
        cluster8(-0.65, 0.4),
        cluster8(-0.1, 0.6),
    ],
    0x987123: [
        cluster6(-0.75, -0.15),
        cluster6(-0.25, -0.85),
        cluster6(-0.55, -0.45),
        cluster6(-0.85, -0.90),
        cluster8(-0.50, -0.15),
        cluster8(-0.80, -0.55),
        cluster8(-0.60, -0.70),
        cluster8(-0.20, -0.45),
    ],
    0xface00: [
        cluster6(0.15, -0.75),
        cluster6(0.85, -0.25),
        cluster6(0.35, -0.65),
        cluster6(0.90, -0.85),
        cluster8(0.15, -0.50),
        cluster8(0.55, -0.80),
        cluster8(0.70, -0.60),
        cluster8(0.45, -0.30),
    ],
    0x2c2134: [
        densecluster8(0.05, 0.05),
        densecluster8(-0.25, -0.05),
    ],
    0xa1b2c3: [
        densecluster8(0, -0.2),
    ],
}

print "#include \"pattern.h\""
print "const struct pusher_config pushers[] = {"

for mac, trees in config.items():
    pixel_locations = list(itertools.chain(*trees))
    print "{ 0x%06x, %d, {" % (mac, len(pixel_locations))
    for (x, y) in pixel_locations:
        print "{ %.06f, %.06f }," % (x, y)
    print "} },"

print "};"
print "const size_t pusher_config_count = %d;" % (len(config), )
