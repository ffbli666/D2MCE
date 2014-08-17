SRC = main.c
EXEC = main

LDLIBS += -lpthread

all:$(EXEC)
$(EXEC):$(SRC)
	$(CC) -g -o $(EXEC) $(SRC) $(LDLIBS)

clean:
	-rm -f $(EXEC)  *.elf *.gdb *.o 
