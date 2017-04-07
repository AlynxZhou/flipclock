CC ?= gcc
CFLAGS ?= -std=c11 -lSDL2 -lSDL2_ttf -lSDL2_gfx
OBJECTS := main.o flipclock.o getarg/getarg.o

flipclock : ${OBJECTS}
	${CC} ${CFLAGS} -o flipclock ${OBJECTS}

debug : CFLAGS += -g
debug : ${OBJECTS}
	${CC} ${CFLAGS} -o flipclock_debug ${OBJECTS}

main.o : flipclock.h

flipclock.o : flipclock.h

getarg/getarg.o : getarg/getarg.h

.PHONY : clean
clean :
	-rm -f flipclock flipclock.o ${OBJECTS} flipclock.h.gch getarg/getarg.h.gch a.out

rebuild : clean lab
