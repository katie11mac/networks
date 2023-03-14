CFLAGS=-Wall -pedantic -g -o

testvde: sender stack

stack: stack.c util.o cs431vde.o crc32.o
	gcc $(CFLAGS) $@ $^

sender: sender.c util.o cs431vde.o
	gcc $(CFLAGS) $@ $^

receiver: receiver.c util.o cs431vde.o
	gcc $(CFLAGS) $@ $^

cs431vde.o: cs431vde.c
	gcc -c $(CFLAGS) $@ $^

crc32.o: crc32.c
	gcc -c $(CFLAGS) $@ $^



testutil: hexdump hexread

hexdump: hexdump.c util.o
	gcc $(CFLAGS) $@ $^

hexread: hexread.c util.o
	gcc $(CFLAGS) $@ $^

util.o: util.c
	gcc -c $(CFLAGS) $@ $^

.PHONY: clean
clean: 
	rm -f hexdump hexread receiver sender stack 
