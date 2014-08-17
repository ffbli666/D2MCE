#ifndef __WQUEUE_H_
#define __WQUEUE_H_

//#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct wqueue{
	unsigned short front;			//front index of wqueue
	unsigned short rear;			//rear index of wqueue
	unsigned short use;				//already use count
	unsigned short wqueue_size;		//wqueue size
	pthread_mutex_t lock;
	void **row;			//wqueue table
};


struct wqueue* wqueueCreate(int wqueue_size);
int wqueueDestroy(struct wqueue *wqueue);
void* wqueueFat(struct wqueue *wqueue, unsigned int increase_size);


__inline__ int wqueueIsEmpty(struct wqueue *wqueue);
__inline__ int wqueueIsFull(struct wqueue *wqueue);
int wqueuePush(struct wqueue *wqueue, void *data);
void* wqueuePop(struct wqueue *wqueue);


#endif

