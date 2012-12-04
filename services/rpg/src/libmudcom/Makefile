.PHONY: clean all
all: libmudcom.so

CC=gcc -m32 -Wall -Werror -std=gnu99 -fPIC -c -O2 -pedantic -fstack-protector -Wno-format-security
LD=gcc -m32 -shared

clean:
	rm -f *.o libmudcom.so

telnet.o: telnet.c telnet.h
	$(CC) -c -o telnet.o telnet.c

map.o: telnet.h map.h map.c
	$(CC) -c -o map.o map.c

libmudcom.so: map.o telnet.o
	$(LD) -o libmudcom.so map.o telnet.o
