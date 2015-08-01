SRCS=pattern.c registry.c sample_patterns.c layout.c
HDRS=wire.h pattern.h

.PHONY: all

all: preview-test pusher-test

layout.c: layout.py
	python layout.py > layout.c

preview-test: $(SRCS) $(HDRS) preview-test.c
	cc -g -std=c89 -ansi -pedantic -Wall -Wextra $(SRCS) preview-test.c -o $@ -F/Library/Frameworks -framework SDL2 -framework Cocoa -framework OpenGL

pusher-test: $(SRCS) $(HDRS) pusher-test.c
	cc -std=c89 -ansi -pedantic -D_BSD_SOURCE -Wall -Wextra $(SRCS) pusher-test.c -lpthread -lm -o $@
