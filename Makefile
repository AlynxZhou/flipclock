# Filename: Makefile
# Created by 请叫我喵 Alynx.
# alynx.zhou@gmail.com, http://alynx.xyz/.
CC ?= gcc
CFLAGS ?= -std=c11
LIBS ?= -lSDL2 -lSDL2_ttf
OBJECTS := main.o flipclock.o getarg/getarg.o

flipclock : CFLAGS += -O2
flipclock : ${OBJECTS}
	${CC} ${CFLAGS} ${LIBS} -o flipclock \
	   ${OBJECTS}

.PHONY : install
install:
	install -o root -m 0755 -D flipclock /usr/bin/flipclock
	install -o root -m 0644 -D flipclock.ttf \
		/usr/share/fonts/flipclock.ttf
	install -o root -m 0644 -D flipclock.png \
		/usr/share/pixmaps/flipclock.png
	install -o root -m 0644 -D flipclock.desktop \
		/usr/share/applications/flipclock.desktop

.PHONY : uninstall
uninstall:
	-rm -f /usr/share/applications/flipclock.desktop \
	       /usr/share/fonts/flipclock.ttf \
	       /usr/share/pixmaps/flipclock.png \
	       /usr/bin/flipclock

.PHONY : debug
debug : CFLAGS += -g
debug : clean ${OBJECTS}
	${CC} ${CFLAGS} ${LIBS} -o flipclock_debug ${OBJECTS}

.PHONY : clean
clean :
	-rm -f flipclock flipclock_debug flipclock.o ${OBJECTS} \
	      flipclock.h.gch getarg/getarg.h.gch a.out

.PHONY : rebuild
rebuild : clean flipclock

main.o : flipclock.h

flipclock.o : flipclock.h

getarg/getarg.o : getarg/getarg.h
