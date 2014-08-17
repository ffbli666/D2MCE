#ifndef __QUEUE_H_
#define __QUEUE_H_

//#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct queue{
	unsigned short front;			//front index of queue
	unsigned short rear;			//rear index of queue
	unsigned short use;				//already use count
	unsigned short queue_size;		//queue size
	unsigned short data_size;		//size of row
	pthread_mutex_t lock;
	void **row;			//queue table
};


struct queue* queueCreate(int queue_size, int data_size);
int queueDestroy(struct queue *queue);
void* queueFat(struct queue *queue, unsigned int increase_size);


__inline__ int queueIsEmpty(struct queue *queue);
__inline__ int queueIsFull(struct queue *queue);
//push
int queuePush(struct queue *queue, void *data);
int queuePushSize(struct queue *queue, void *data, int size);
void* queueGetPush(struct queue *queue);
//pop
void* queuePop(struct queue *queue);


#endif

