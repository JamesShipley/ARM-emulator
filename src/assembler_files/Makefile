CC = gcc
CFLAGS = -c -Wall
EXE = assemble

all: ${EXE}

${EXE}: main.o assemble_main.o utils.o
	${CC} main.o assemble_main.o utils.o -o assemble

main.o: main.h main.c
	 ${CC} ${CFLAGS} main.c

assemble_main.o: assemble_main.h assemble_main.c
	 ${CC} ${CFLAGS} assemble_main.c

utils.o: common_structs.h utils.h utils.c
	${CC} ${CFLAGS} utils.c

clean:
	rm *.o assemble
