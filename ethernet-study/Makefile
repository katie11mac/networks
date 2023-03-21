CFLAGS=-Wall -pedantic -g -o

binary_backoff: binary_backoff.c
	gcc $(CFLAGS) $@ $^

.PHONY: clean
clean: 
	rm -f binary_backoff
