#ifndef __SYNCHRONIZATION_H_
#define __SYNCHRONIZATION_H_

#include <semaphore.h>
#include <pthread.h>
#include "network.h"
#include "memory.h"

struct synchronization{
	char acquire;
	struct table *release;	//necessary wait invalid ok
	pthread_mutex_t lock;
};

struct sync_wait{
	int seq_number;
	sem_t *sem;
};

__inline__ int sync_init();
__inline__ int sync_destroy();

int addWait(unsigned int seq_number);
int addWaitSem(sem_t *sem);

__inline__ int acquire();
__inline__ int isAcquire();
int release();

#endif

