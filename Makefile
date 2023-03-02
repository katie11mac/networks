CFLAGS=-Wall -pedantic -g -o

testutil: hexdump hexread

hexdump: hexdump.c util.o
	gcc $(CFLAGS) $@ $< util.o

hexread: hexread.c util.o
	gcc $(CFLAGS) $@ $< util.o

util.o: util.c
	gcc -c $(CFLAGS) $@ $^

.PHONY: clean
clean: 
	rm -f hexdump hexread util
