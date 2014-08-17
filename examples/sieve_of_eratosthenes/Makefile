SRC = findprime.c findprime-cache.c
EXEC = findprime findprime-cache

LDLIBS += -lpthread

all:$(EXEC)
$(EXEC):$(SRC)

findprime:findprime.c
	$(CC) -g -o findprime findprime.c -lpthread

findprime-cache:findprime-cache.c
	$(CC) -g -o findprime-cache findprime-cache.c -lpthread

clean:
	-rm -f $(EXEC)  *.elf *.gdb *.o 
