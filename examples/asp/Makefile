ALL = asp_data asp_print  asp_sequence asp_d2mce asp_d2mce_m

DATA_EXEC = asp_data
DATA_OBJS = asp_data.o

SIGNAL_EXEC = asp_sequence
SIGNAL_OBJS = asp_sequence.o

D2MCE_EXEC = asp_d2mce
D2MCE_OBJS = asp_d2mce.o

PRINT_EXEC = asp_print
PRINT_OBJS = asp_print.o

MEXEC = asp_d2mce_m
MOBJS = asp_d2mce_m.o


CFLAGS = -Wall -g -fopenmp
LIBS += -lpthread -fopenmp
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

${MEXEC}: ${MOBJS}
	${CC} ${CFALGS} -o $@ ${MOBJS} ${LIBS} ${D2MCE}


*.o:*.c

clean:
	-rm -f $(ALL)  *.elf *.gdb *.o *.data
