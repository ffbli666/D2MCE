# Description:
#    barrier.h
# 
#  Copyright (C) 2006- by EPS(Embedded and Parallel Systems Lab) @ NTUT CSIE
# 
#  Date: $Date: 2007/12/20 13:09:05 $
#  Version: $Revision: 1.5 $
# 
#  History:
# 
#  $Log: Makefile,v $
#  Revision 1.5  2007/12/20 13:09:05  ffbli
#  add log
#
# 
ALL_OBJS = memory.o socket.o common.o ctrlThread.o dataThread.o hashtable.o receiver.o sender.o network.o table.o thread.o queue.o wqueue.o stack.o pointtable.o barrier.o mutexlock.o semaphore.o sharememory.o synchronization.o finalize.o join.o manager.o exit.o

D2MCE_OBJS = d2mce.o
D2MCE_LIBS = libd2mce.a

D2MCED_EXEC = d2mced 
D2MCED_OBJS = d2mced.o common.o table.o socket.o

ALL = $(ALL_OBJS) $(D2MCE_LIBS) $(D2MCED_EXEC) 

CFLAGS = -Wall -g
LIBS += -lpthread
#CC=arm-unknown-linux-gnu-gcc

all:${ALL}  
	@echo -ne 'Done.\n'

#${D2MCE_OBJS}: d2mce.c ${ALL_OBJS}
#	${CC} -c $(CFLAGS) -o $@ d2mce.c ${ALL_OBJS} ${LIBS}

$(D2MCE_LIBS):$(D2MCE_OBJS)
	ar rcv $@ $(D2MCE_OBJS) ${ALL_OBJS} 

$(D2MCED_EXEC): $(D2MCED_OBJS)
	${CC} $(CFLAGS) -o $@ $(D2MCED_OBJS) ${LIBS}

*.o : *.c

install:
	cp d2mced ../bin
	cp $(D2MCE_LIBS) ../lib
	mkdir $(HOME)/.d2mce_conf -p
	cp ../conf/d2mce.conf $(HOME)/.d2mce_conf/d2mce.conf

clean:
	-rm -f $(ALL)  *.elf *.gdb *.o 

