SRCS = pattern.c registry.c sample_patterns.c layout.c color_util.c
HDRS = wire.h pattern.h color_util.h clock.h
LIBS = -lm -lpthread

OS := $(shell uname)
ifeq ($(OS),Linux)
    SRCS += clock_midi.c
    LIBS += -lasound
else
    SRCS += clock_sim.c
endif

.PHONY: all clean

all: preview-test pusher-test

clean:
	rm -f pusher-test preview-test

layout.c: layout.py
	python layout.py > layout.c

preview-test: $(SRCS) $(HDRS) preview-test.c
	cc -g -O2 -Wall -Wextra $(SRCS) preview-test.c $(LIBS) -o $@ -F/Library/Frameworks -framework SDL2 -framework Cocoa -framework OpenGL

pusher-test: $(SRCS) $(HDRS) pusher-test.c
	cc -g -O2 -Wall -Wextra -D_BSD_SOURCE $(SRCS) pusher-test.c $(LIBS) -o $@ -flto
