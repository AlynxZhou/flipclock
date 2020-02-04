# Alynx Zhou <alynx.zhou@gmail.com> (https://alynx.moe/)

CC ?= gcc
CFLAGS ?= -std=c11
LIBS ?= -lSDL2 -lSDL2_ttf
OBJECTS := main.o flipclock.o getarg.o

flipclock: CFLAGS += -O2
flipclock: ${OBJECTS}
	${CC} ${CFLAGS} ${LIBS} -o flipclock \
	   ${OBJECTS}

.PHONY: install
install:
	install -o root -m 0755 -D flipclock ${DESTDIR}/usr/bin/flipclock
	install -o root -m 0644 -D flipclock.ttf \
		${DESTDIR}/usr/share/fonts/flipclock.ttf
	install -o root -m 0644 -D flipclock.png \
		${DESTDIR}/usr/share/pixmaps/flipclock.png
	install -o root -m 0644 -D flipclock.desktop \
		${DESTDIR}/usr/share/applications/flipclock.desktop

.PHONY: uninstall
uninstall:
	-rm -f ${DESTDIR}/usr/share/applications/flipclock.desktop \
	       ${DESTDIR}/usr/share/fonts/flipclock.ttf \
	       ${DESTDIR}/usr/share/pixmaps/flipclock.png \
	       ${DESTDIR}/usr/bin/flipclock

.PHONY: debug
debug: CFLAGS += -g
debug: clean ${OBJECTS}
	${CC} ${CFLAGS} ${LIBS} -o flipclock_debug ${OBJECTS}

.PHONY: clean
clean:
	-rm -f flipclock flipclock_debug flipclock.o ${OBJECTS} \
	      flipclock.h.gch getarg.h.gch a.out

.PHONY: rebuild
rebuild: clean flipclock

main.o: flipclock.h

flipclock.o: flipclock.h

getarg.o: getarg.h
