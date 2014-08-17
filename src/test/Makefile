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
EXIT_EXEC = exit
EXIT_OBJS = exit.o

EVENT_EXEC = event
EVENT_OBJS = event.o

ALLLOAD_EXEC = allload
ALLLOAD_OBJS = allload.o

ALLLOAD_UP_EXEC = allload_update
ALLLOAD_UP_OBJS	= allload_update.o

IMAIN_EXEC = imain
IMAIN_OBJS = imain.o

IBARRIER_EXEC = ibarrier
IBARRIER_OBJS = ibarrier.o

IMUTEX_EXEC = imutex
IMUTEX_OBJS = imutex.o

ISEM_EXEC = isem
ISEM_OBJS = isem.o

IHOME_EXEC = iamhome
IHOME_OBJS = iamhome.o

PTHREAD_EXEC = pthread
PTHREAD_OBJS = pthread.o

MSTORE_EXEC = mstore
MSTORE_OBJS = mstore.o

MLOAD_EXEC = mload
MLOAD_OBJS = mload.o

LS_EXEC =loadstore
LS_OBJS =loadstore.o

FETCH_EXEC = fetch
FETCH_OBJS = fetch.o

STORE_EXEC = store
STORE_OBJS = store.o

LOAD_EXEC = load
LOAD_OBJS = load.o

MALLOC_EXEC = malloc
MALLOC_OBJS = malloc.o

SEM_EXEC = sem
SEM_OBJS = sem.o

MUTEX_EXEC = mutex
MUTEX_OBJS = mutex.o

BARRIER_EXEC = barrier
BARRIER_OBJS = barrier.o

JGID_EXEC = join_gid
JGID_OBJS = join_gid.o

JOIN_EXEC = join
JOIN_OBJS = join.o

PROBE_EXEC = join_probe
PROBE_OBJS = join_probe.o

TEST_EXEC = testqueue
TEST_OBJS = testqueue.o ../queue.o

TEST_TABLE = testtable
TEST_TOBJS = testtable.o ../table.o

TEST_HASH = testhash
TEST_HASH_OBJ = testhash.o ../hashtable.o

TEST_HASH2 = hash2
TEST_HASH2_OBJ = hash2.o ../hashtable.o

TEST_POINT = pointtable
TEST_POINT_OBJ = pointtable.o ../pointtable.o

TEST_MEM = testmemory
TEST_MEM_OBJ = testmemory.o ../memory.o ../stack.o


DIFF_EXEC = diff
DIFF_OBJS = diff.o ../sharememory.o


ALL = ${TEST_MEM} ${TEST_EXEC} ${TEST_TABLE} ${TEST_HASH} ${TEST_HASH2}  ${TEST_POINT} ${JOIN_EXEC} ${PROBE_EXEC} ${JGID_EXEC} ${BARRIER_EXEC} ${MUTEX_EXEC} ${SEM_EXEC} ${MALLOC_EXEC} ${LOAD_EXEC} ${STORE_EXEC} ${FETCH_EXEC} ${LS_EXEC} ${MLOAD_EXEC} ${MSTORE_EXEC} ${DIFF_EXEC} ${PTHREAD_EXEC} ${IHOME_EXEC} ${ALLLOAD_EXEC} ${ALLLOAD_UP_EXEC} ${IBARRIER_EXEC} ${IMUTEX_EXEC} ${ISEM_EXEC} ${EVENT_EXEC} ${IMAIN_EXEC} ${EXIT_EXEC}

CFLAGS = -Wall -g
LIBS += -lpthread
D2MCE = -ld2mce -L../
#CC=arm-unknown-linux-gnu-gcc

all:${ALL}
	@echo -ne 'Done.\n'
${PTHREAD_EXEC}: ${PTHREAD_OBJS}
	${CC} ${CFALGS} -o $@ ${PTHREAD_OBJS} ${LIBS} ${D2MCE}

${JGID_EXEC}: ${JGID_OBJS}
	${CC} ${CFALGS} -o $@ ${JGID_OBJS} ${LIBS} ${D2MCE}

${PROBE_EXEC}: ${PROBE_OBJS}
	${CC} ${CFALGS} -o $@ ${PROBE_OBJS} ${LIBS} ${D2MCE}

${JOIN_EXEC}: ${JOIN_OBJS}
	${CC} ${CFALGS} -o $@ ${JOIN_OBJS} ${LIBS} ${D2MCE}

${BARRIER_EXEC}: ${BARRIER_OBJS}
	${CC} ${CFALGS} -o $@ ${BARRIER_OBJS} ${LIBS} ${D2MCE}

${MUTEX_EXEC}: ${MUTEX_OBJS}
	${CC} ${CFALGS} -o $@ ${MUTEX_OBJS} ${LIBS} ${D2MCE}

${SEM_EXEC}: ${SEM_OBJS}
	${CC} ${CFALGS} -o $@ ${SEM_OBJS} ${LIBS} ${D2MCE}

${MALLOC_EXEC}: ${MALLOC_OBJS}
	${CC} ${CFALGS} -o $@ ${MALLOC_OBJS} ${LIBS} ${D2MCE}

${LOAD_EXEC}: ${LOAD_OBJS}
	${CC} ${CFALGS} -o $@ ${LOAD_OBJS} ${LIBS} ${D2MCE}

${MLOAD_EXEC}: ${MLOAD_OBJS}
	${CC} ${CFALGS} -o $@ ${MLOAD_OBJS} ${LIBS} ${D2MCE}

${STORE_EXEC}: ${STORE_OBJS}
	${CC} ${CFALGS} -o $@ ${STORE_OBJS} ${LIBS} ${D2MCE}

${MSTORE_EXEC}: ${MSTORE_OBJS}
	${CC} ${CFALGS} -o $@ ${MSTORE_OBJS} ${LIBS} ${D2MCE}

${FETCH_EXEC}: ${FETCH_OBJS}
	${CC} ${CFALGS} -o $@ ${FETCH_OBJS} ${LIBS} ${D2MCE}

${LS_EXEC}: ${LS_OBJS}
	${CC} ${CFALGS} -o $@ ${LS_OBJS} ${LIBS} ${D2MCE}


${TEST_EXEC}: ${TEST_OBJS}
	${CC} ${CFLAGS} -o $@ ${TEST_OBJS} ${LIBS}

${TEST_TABLE}: ${TEST_TOBJS}
	${CC} ${CFLAGS} -o $@ ${TEST_TOBJS} ${LIBS}

${TEST_HASH}: ${TEST_HASH_OBJ}
	${CC} ${CFLAGS} -o $@ ${TEST_HASH_OBJ} ${LIBS}

${TEST_HASH2}: ${TEST_HASH2_OBJ}
	${CC} ${CFLAGS} -o $@ ${TEST_HASH2_OBJ} ${LIBS}

${TEST_POINT}: ${TEST_POINT_OBJ}
	${CC} ${CFLAGS} -o $@ ${TEST_POINT_OBJ} ${LIBS}

${TEST_MEM}: ${TEST_MEM_OBJ}
	${CC} ${CFLAGS} -o $@ ${TEST_MEM_OBJ} ${LIBS}


${DIFF_EXEC}: ${DIFF_OBJS}
	${CC} ${CFLAGS} -o $@ ${DIFF_OBJS} ${LIBS}  ${D2MCE}

${IHOME_EXEC}: ${IHOME_OBJS}
	${CC} ${CFLAGS} -o $@ ${IHOME_OBJS} ${LIBS} ${D2MCE}

${IBARRIER_EXEC}: ${IBARRIER_OBJS}
	${CC} ${CFLAGS} -o $@ ${IBARRIER_OBJS} ${LIBS} ${D2MCE}

${IMUTEX_EXEC}: ${IMUTEX_OBJS}
	${CC} ${CFLAGS} -o $@ ${IMUTEX_OBJS} ${LIBS} ${D2MCE}

${ISEM_EXEC}: ${ISEM_OBJS}
	${CC} ${CFLAGS} -o $@ ${ISEM_OBJS} ${LIBS} ${D2MCE}

${IMAIN_EXEC}: ${IMAIN_OBJS}
	${CC} ${CFLAGS} -o $@ ${IMAIN_OBJS} ${LIBS} ${D2MCE}

${ALLLOAD_EXEC}: ${ALLLOAD_OBJS}
	${CC} ${CFALGS} -o $@ ${ALLLOAD_OBJS} ${LIBS} ${D2MCE}

${ALLLOAD_UP_EXEC}: ${ALLLOAD_UP_OBJS}
	${CC} ${CFALGS} -o $@ ${ALLLOAD_UP_OBJS} ${LIBS} ${D2MCE}

${EVENT_EXEC}: ${EVENT_OBJS}
	${CC} ${CFALGS} -o $@ ${EVENT_OBJS} ${LIBS} ${D2MCE}

${EXIT_EXEC}: ${EXIT_OBJS}
	${CC} ${CFALGS} -o $@ ${EXIT_OBJS} ${LIBS} ${D2MCE}


*.o : *.c

clean:
	-rm -f ${ALL}  *.elf *.gdb *.o ../waitqueue.o ../table.o

