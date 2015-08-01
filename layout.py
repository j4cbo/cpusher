from math import sin, cos, pi

def emit(x, y):
    print "{ %.06f, %.06f }," % (x, y)

def draw_cluster(arms, leds_per_arm, led_distance, x, y):
    for rot in range(arms):
        for dist in range(leds_per_arm):
            emit(x + cos(rot * pi * 2 / arms) * led_distance * (dist + 1.5),
                 y + sin(rot * pi * 2 / arms) * led_distance * (dist + 1.5))

def draw_cluster6(x, y):
    led_distance = .03
    for rot in range(6):
        for dist in (0, 2, 3, 1):
            emit(x + cos(rot * pi * 2 / 6) * led_distance * (dist + 1.5),
                 y + sin(rot * pi * 2 / 6) * led_distance * (dist + 1.5))

def draw_cluster8(x, y):
    led_distance = .04
    for rot in range(8):
        for dist in (0, 2, 1):
            emit(x + cos(rot * pi * 2 / 8) * led_distance * (dist + 1.5),
                 y + sin(rot * pi * 2 / 8) * led_distance * (dist + 1.5))


print "struct xy { double x; double y; };"
print "struct xy tree_centers[] = {"
draw_cluster6(-0.3, 0.7)
draw_cluster6(-0.5, -0.7)
draw_cluster6(-0.8, 0.2)
draw_cluster6(-0.1, -0.4)
draw_cluster8(0.6, -0.2)
draw_cluster8(0.5, 0.6)
draw_cluster8(-0.2, 0.3)
draw_cluster8(0.2, -0.8)
print "};"
