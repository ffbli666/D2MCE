#ifndef __THREAD_H_
#define __THREAD_H_

#include <semaphore.h>
#include <pthread.h>
#include <sys/signal.h>
#include "define.h"
#include "table.h"
#include "receiver.h"
#include "sender.h"
#include "ctrlThread.h"
#include "dataThread.h"

/*
  *	Thread Manager use
  */


enum coordinator_tid{
	RECEIVER_TID = 0,
	SENDER_TID,
	CTRL_TID,	
	DATA_TID
};


//role_info
struct role_recvbuf{
	char stackbuf;
	void *buf;
};

struct role_info{
//	char init;
	unsigned int count;
	sem_t sem;
	pthread_mutex_t lock;
	struct wqueue *wqueue;
};

sem_t g_thread_sem;

struct thread{
	pthread_t pid;
	enum coordinator_tid who;
	unsigned short enable;
	void *(*start)(void *);
	int (*close)();
};

int thread_init();
int thread_destroy();
//int thread_closeAll();
int threadStart(int index);
int threadJoin(int index);
int threadKill(int index);

#endif
