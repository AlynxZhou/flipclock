CC ?= gcc
CFLAGS ?= -std=c11 -lSDL2 -lSDL2_ttf -lSDL2_gfx
OBJECTS := main.o flipclock.o getarg/getarg.o

flipclock : ${OBJECTS}
	${CC} ${CFLAGS} -o flipclock ${OBJECTS}

install:
	install -o root -m 0755 -D flipclock /usr/bin/flipclock
	install -o root -m 0644 -D flipclock.ttf /usr/share/fonts/flipclock.ttf
	install -o root -m 0644 -D flipclock.png /usr/share/pixmaps/flipclock.png
	install -o root -m 0644 -D flipclock.desktop /usr/share/applications/screensavers/flipclock.desktop

uninstall:
	-rm -f /usr/share/applications/screensavers/flipclock.desktop\
	       /usr/share/fonts/flipclock.ttf\
	       /usr/share/pixmaps/flipclock.png\
	       /usr/bin/flipclock\

debug : CFLAGS += -g
debug : ${OBJECTS}
	${CC} ${CFLAGS} -o flipclock_debug ${OBJECTS}

main.o : flipclock.h

flipclock.o : flipclock.h

getarg/getarg.o : getarg/getarg.h

.PHONY : clean
clean :
	-rm -f flipclock flipclock.o ${OBJECTS} flipclock.h.gch getarg/getarg.h.gch a.out

rebuild : clean flipclock
