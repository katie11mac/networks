CFLAGS=-Wall -pedantic -g -o

testvde: testing-sender stack

stack: stack.c util.o cs431vde.o crc32.o ip_checksum.o
	gcc $(CFLAGS) $@ $^

testing-sender: testing-sender.c util.o cs431vde.o crc32.o ip_checksum.o
	gcc $(CFLAGS) $@ $^

receiver: receiver.c util.o cs431vde.o
	gcc $(CFLAGS) $@ $^



testutil: hexdump hexread

hexdump: hexdump.c util.o
	gcc $(CFLAGS) $@ $^

hexread: hexread.c util.o
	gcc $(CFLAGS) $@ $^



%.o: %.c
	gcc -c $(CFLAGS) $@ $^

.PHONY: clean
clean: 
	rm -f hexdump hexread receiver testing-sender stack *.o 
