test: test.c registry.c wire.h
	cc -std=c89 -ansi -pedantic -Wall -Wextra test.c registry.c -o test
