SRC = jacobi.c jacobi-d2mce.c jacobi-d2mce-dynamic.c
EXEC = jacobi jacobi-d2mce jacobi-d2mce_m

MEXEC = jacobi-d2mce_m
MOBJS = jacobi-d2mce_m.o


CFLAGS = -Wall -g -fopenmp
LDLIBS += -lpthread -fopenmp
D2MCE = -ld2mce -L../../src/ -I../../src/

all:$(EXEC)
$(EXEC):$(SRC)

jacobi:jacobi.o
	$(CC) ${CFALGS} -o $@ jacobi.o

jacobi-d2mce:jacobi-d2mce.o
	$(CC) ${CFALGS} -o $@ jacobi-d2mce.o $(LDLIBS) ${D2MCE}

jacobi-d2mce-dynamic: jacobi-d2mce-dynamic.o
	$(CC) ${CFALGS} -o $@ jacobi-d2mce-dynamic $(LDLIBS) ${D2MCE}

${MEXEC}: ${MOBJS}
	${CC} ${CFALGS} -o $@ ${MOBJS} ${LDLIBS} ${D2MCE}



*.o:*.c

clean:
	-rm -f $(EXEC)  *.elf *.gdb *.o *.data
