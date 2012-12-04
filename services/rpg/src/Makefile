.PHONY: clean all run pack deploy strip

CC=gcc -m32 -Wall -Werror -std=gnu99 -c -O2 -pedantic -fstack-protector -Wno-format-security
LD=gcc -m32 -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -O2 -pthread


OBJECTS=main.o game_engine.o rpc.o store.o npc_parrot.o npc_nun.o npc_achmed.o npc_librarian.o npc_vendor.o

all: rpg

libmudcom.so:
	make -C libmudcom
	cp libmudcom/libmudcom.so libmudcom.so

rpg: $(OBJECTS) libmudcom.so
	$(LD) -o rpg $(OBJECTS) -Llibmudcom -lmudcom

main.o: main.c
	$(CC) -o main.o main.c

%.o: %.c %.h
	$(CC) -o $@ $<

clean:
	rm -f *.o rpg libmudcom.so rpg-strip ctfbundle.tar.xz
	make -C libmudcom clean

pack:
	tar -cvJf rpg.tar.xz *.c *.h Makefile

strip: rpg
	cp rpg rpg-strip
	strip -N debugStore rpg-strip
	strip -w -N "achmed*" rpg-strip
	strip -w -N "parrot*" rpg-strip
	strip -w -N "librarian*" rpg-strip
	strip -w -N "vendor*" rpg-strip
	strip -w -N "nun*" rpg-strip

bundle: rpg-strip libmudcom.so
	tar -cvJf ctfbundle.tar.xz libmudcom/*.h libmudcom/*.c libmudcom/Makefile rpg-strip libmudcom.so api/.dungeonapy.py.swp

deploy: strip rpg
	scp -r rpg run libmudcom.so libmudcom/ vulnbox:/etc/vservers/rpg/vdir/home/rpg/

run: rpg
	LD_LIBRARY_PATH="." ./rpg
