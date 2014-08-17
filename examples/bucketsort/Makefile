ALL =is_data is_signal is_d2mce

DATA_EXEC = is_data
DATA_OBJS = is_data.o

SIGNAL_EXEC = is_signal
SIGNAL_OBJS = is_signal.o

D2MCE_EXEC = is_d2mce
D2MCE_OBJS = is_d2mce.o

PRINT_EXEC = is_print
PRINT_OBJS = is_print.o

CFLAGS = -Wall -g
LIBS += -lpthread
D2MCE = -ld2mce -L../../src/

all:${ALL}

${DATA_EXEC}: ${DATA_OBJS}
	${CC} ${CFALGS} -o $@ ${DATA_OBJS} ${LIBS} 

${SIGNAL_EXEC}: ${SIGNAL_OBJS}
	${CC} ${CFALGS} -o $@ ${SIGNAL_OBJS} ${LIBS}

${D2MCE_EXEC}: ${D2MCE_OBJS}
	${CC} ${CFALGS} -o $@ ${D2MCE_OBJS} ${LIBS} ${D2MCE}

${PRINT_EXEC}: ${PRINT_OBJS}
	${CC} ${CFALGS} -o $@ ${PRINT_OBJS} ${LIBS}


*.o:*.c

clean:
	-rm -f $(ALL)  *.elf *.gdb *.o 
