CFLAGS=-Wall -pedantic -g -o

ethernet-study: binary_backoff hubs_vs_switches

binary_backoff: binary_backoff.c
	gcc $(CFLAGS) $@ $^

hubs_vs_switches: hubs_vs_switches.c
	gcc $(CFLAGS) $@ $^

.PHONY: clean
clean: 
	rm -f binary_backoff hubs_vs_switches
