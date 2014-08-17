SRC = motion.c
EXEC = motion
OBJS = motion.o

CFLAGS = -Wall -g 
LIBS += -lpthread -lm
D2MCE = -ld2mce -L../../src/ 


all:$(EXEC)
${EXEC}: ${OBJS}
	${CC} ${CFALGS} -o $@ ${OBJS} ${LIBS} ${D2MCE}

*.o:*.c

clean:
	-rm -f $(EXEC)  *.elf *.gdb *.o 
