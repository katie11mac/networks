CFLAGS=-Wall -pedantic -o

testutil: hexdump hexread

hexdump: hexdump.c util
	gcc $(CFLAGS) $@ $<

hexread: hexread.c util
	gcc $(CFLAGS) $@ $<

util: util.c
	gcc $(CFLAGS) $@ $^

.PHONY: clean
clean: 
	rm -f hexdump hexread util
