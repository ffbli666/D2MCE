EXEC = sm_snake2
OBJS = sm_snake2.o

EXEC2 = sm_snake2_up
OBJS2 = sm_snake2_up.o


CFLAGS = -Wall -g
LIBS += -lpthread -lncurses
D2MCE = -ld2mce -L../../src/

all:$(EXEC) ${EXEC2}

${EXEC}: ${OBJS}
	${CC} ${CFALGS} -o $@ ${OBJS} ${LIBS} ${D2MCE}

${EXEC2}: ${OBJS2}
	${CC} ${CFALGS} -o $@ ${OBJS2} ${LIBS} ${D2MCE}


*.o:*.c

clean:
	-rm -f $(EXEC)  *.elf *.gdb *.o 
