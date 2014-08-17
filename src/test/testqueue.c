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

void *testq(void *argv){
	struct queue *abc;
	abc = queueCreate(100, sizeof(struct role_queue));
	printf("queue create ok\n");
	while(e!=1);
	queueDestroy(abc);
	printf("queue destroy ok\n");
	return NULL;
}
int main(int argc, char *argv[]){
//	struct queue *queue;
	struct a a;
	char str[7];
	void *buf;
	int i,j;
	double time;
	pthread_t pid;
	e=0;
	pthread_create(&pid, NULL, testq, NULL);

	a.q=queueCreate( 100, 7);
  	if(queueIsEmpty(a.q))
		printf("empty\n");
	if(!queueIsFull(a.q))
		printf("not full\n");
	for(i=0;i<10;i++){
		snprintf(str,7,"char%d",i);
		queuePush(a.q, str);
	}

    if(!queueIsEmpty(a.q))
        printf("not empty\n");
    if(queueIsFull(a.q))
        printf("full\n");
	a.q = queueFat(a.q, a.q->queue_size);

	if(!queuePush(a.q, str))
		printf("full can't push\n");
	a.q = queueFat(a.q, a.q->queue_size);
	a.q = queueFat(a.q, a.q->queue_size);
	for(i=0;i<10;i++){
        buf = queuePop(a.q);
		printf("%s\n", (char*)buf);
    }
	if(!queuePop(a.q))
		printf("empty can't pop\n");
/*	
    for(i=0;i<10;i++){
        snprintf(str,7,"char%d",i);
        queuePush(queue, str);
    }
    for(i=0;i<10;i++){
        buf = queuePop(queue);
        printf("%s\n", (char*)buf);
    }
*/
	time = -gettime();
	for(j=0;j<100000;j++){
		for(i=0;i<10;i++){
			queuePush(a.q, "a");
			buf = queuePop(a.q);
		}
	}
	time += gettime();
	printf("time %f\n", time);
	time = -gettime();
    for(j=0;j<100000;j++){
        for(i=0;i<10;i++){
            queuePush(a.q, "a");
        }
        for(i=0;i<10;i++){
            buf = queuePop(a.q);
        }
    }
    time += gettime();
    printf("time %f\n", time);
	e=1;
	pthread_join(pid, NULL);
	queueDestroy(a.q);	
	return 1;
}
