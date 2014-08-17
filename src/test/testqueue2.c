#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../queue.h"
#include "time.c"


int e;

struct a{
	struct queue *q;
};
struct role_queue{
	ssize_t recv_size;
	char buf[1024];
};

int main(int argc, char *argv[]){
//	struct queue *queue;
	struct a a;
	char str[7];
	void *buf;
	int i,j;
	double time;
	pthread_t pid;
	e=0;

	a.q=queueCreate( 10, 7);
  	if(queueIsEmpty(a.q))
		printf("empty\n");
	if(!queueIsFull(a.q))
		printf("not full\n");
	for(i=0;i<10;i++){
		snprintf(str,7,"char%d",i);
		queuePush(a.q, str);
	}
	for(i=0;i<5;i++)
		queuePop(a.q);
    for(i=10;i<15;i++){
        snprintf(str,7,"char%d",i);
        queuePush(a.q, str);
    }
    for(i=0;i<a.q->queue_size;i++){
        printf("%s\n", a.q->row[i]);
    }
	printf("----------------------------------------------------------\n");
    if(!queueIsEmpty(a.q))
        printf("not empty\n");
    if(queueIsFull(a.q))
        printf("full\n");
	a.q = queueFat(a.q, a.q->queue_size);
	for(i=0;i<a.q->queue_size;i++){
		printf("%s\n", a.q->row[i]);
	}
	printf("----------------------------------------------------------\n");
    for(i=30;i<35;i++){
        snprintf(str,7,"char%d",i);
        queuePush(a.q, str);
    }
    for(i=0;i<a.q->queue_size;i++){
        printf("%s\n", a.q->row[i]);
    }
	printf("----------------------------------------------------------\n");
	for(i=0;i<20;i++)
        queuePop(a.q);
    for(i=70;i<90;i++){
        snprintf(str,7,"char%d",i);
        queuePush(a.q, str);
    }
    for(i=0;i<a.q->queue_size;i++){
        printf("%s\n", a.q->row[i]);
    }


	return 1;
}
