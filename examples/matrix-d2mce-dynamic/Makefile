ALL = matrix matrix_m

SRC = matrix.c
EXEC = matrix
OBJS = matrix.o

MEXEC = matrix_m
MOBJS = matrix_m.o

CFLAGS = -Wall -g -fopenmp

LIBS += -lpthread -fopenmp

D2MCE = -ld2mce -L../../src/
#CC=arm-unknown-linux-gnu-gcc

all:$(ALL)
${EXEC}: ${OBJS}
	${CC} ${CFALGS} -o $@ ${OBJS} ${LIBS} ${D2MCE}

${MEXEC}: ${MOBJS}
	${CC} ${CFALGS} -o $@ ${MOBJS} ${LIBS} ${D2MCE}


*.o:*.c

clean:
	-rm -f $(EXEC)  *.elf *.gdb *.o 
