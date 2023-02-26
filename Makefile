CFLAGS=-Wall -pedantic -o
util: util.c
	gcc $(CFLAGS) $@ $^

.PHONY: clean
	clean rm -f util
