ALL = sor_data sor_print sor_sequence sor_d2mce

DATA_EXEC = sor_data
DATA_OBJS = sor_data.o

SIGNAL_EXEC = sor_sequence
SIGNAL_OBJS = sor_sequence.o

D2MCE_EXEC = sor_d2mce
D2MCE_OBJS = sor_d2mce.o

PRINT_EXEC = sor_print
PRINT_OBJS = sor_print.o

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


*.o:*.c

clean:
	-rm -f $(ALL)  *.elf *.gdb *.o *.data
