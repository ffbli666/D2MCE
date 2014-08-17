ALL = lu_data lu_print  lu_signal lu_d2mce

DATA_EXEC = lu_data
DATA_OBJS = lu_data.o

SIGNAL_EXEC = lu_signal
SIGNAL_OBJS = lu_signal.o

D2MCE_EXEC = lu_d2mce
D2MCE_OBJS = lu_d2mce.o

PRINT_EXEC = lu_print
PRINT_OBJS = lu_print.o

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
