from __future__ import division
from math import sin, cos, pi
import json
import itertools

# Generate the pattern for a 6-arm (4-pixel-per-arm) tree.
def cluster6(x, y, rotation):
    rotation = rotation or 0
    pixel_spacing = .015
    inner_pixel_radius = 1.5 * pixel_spacing
    num_arms = 6
    # for each arm...
    for arm in range(num_arms):
        theta = ((arm / num_arms) + (rotation / 360)) * (2 * pi)
        # for each pixel in the arm...
        # this is out of order to reflect how we wire the pixels out of order (so the
        # cables will reach): first innermost, then second-outermost, then outermost,
        # then second-innermost, then on to the next arm.
        for pixel_index in (0, 2, 3, 1):
            distance_from_center = inner_pixel_radius + (pixel_index * pixel_spacing)
            yield (x + cos(theta) * distance_from_center, y + sin(theta) * distance_from_center)

# Generate the pattern for an 8-arm (3-pixel-per-arm) tree.
def cluster8(x, y, rotation):
    rotation = rotation or 0
    pixel_spacing = .02
    inner_pixel_radius = 1.5 * pixel_spacing
    num_arms = 8
    # for each arm...
    for arm in range(num_arms):
        theta = ((arm / num_arms) + (rotation / 360)) * (2 * pi)
        # for each pixel in the arm...
        # again, this is out of order to reflect the pixel wiring order: 0 (inner),
        # then 2 (outer), then 1 (middle), then on to the next arm
        for pixel_index in (0, 2, 1):
            distance_from_center = inner_pixel_radius + (pixel_index * pixel_spacing)
            yield (x + cos(theta) * distance_from_center, y + sin(theta) * distance_from_center)

def densecluster8(x, y):
    led_distance = .001
    for rot in range(8):
        for dist in range(160):
            yield(x + cos(rot * pi * 2 / 8) * led_distance * (dist + 10),
                  y + sin(rot * pi * 2 / 8) * led_distance * (dist + 10))

config = json.load(file("mapping.json"))

print "#include \"pattern.h\""
print "const struct pusher_config pushers[] = {"

for mac, trees in config.items():
    mac = int(mac, 16)
    pixel_locations = []
    for tree in trees:
        if tree['ignore']:
            pixel_locations += [ (999, 999) ] * 24
        elif tree['type'] == 'cluster6':
            x, y = tree['center']
            rotation = tree['rotation']
            pixel_locations += cluster6(x, y, rotation)
        elif tree['type'] == 'cluster8':
            x, y = tree['center']
            rotation = tree['rotation']
            pixel_locations += cluster6(x, y, rotation)
        elif tree['type'] == 'densecluster8':
            x, y = tree['center']
            rotation = tree['rotation']
            pixel_locations += densecluster8(x, y)

    print "{ 0x%06x, %d, {" % (mac, len(pixel_locations))
    for (x, y) in pixel_locations:
        print "{ %.06f, %.06f }," % (x, y)
    print "} },"

print "};"
print "const size_t pusher_config_count = %d;" % (len(config), )
