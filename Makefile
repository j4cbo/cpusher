SRCS=pattern.c registry.c sample_patterns.c layout.c color_util.c
HDRS=wire.h pattern.h color_util.h clock.h

.PHONY: all

all: preview-test pusher-test

layout.c: layout.py
	python layout.py > layout.c

preview-test: $(SRCS) $(HDRS) preview-test.c clock_sim.c
	cc -g -Wall -Wextra $(SRCS) preview-test.c clock_sim.c -o $@ -F/Library/Frameworks -framework SDL2 -framework Cocoa -framework OpenGL

pusher-test: $(SRCS) $(HDRS) pusher-test.c
	cc -g -Wall -Wextra -D_BSD_SOURCE $(SRCS) pusher-test.c -lpthread -lm -o $@
