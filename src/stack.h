#ifndef __STACK_H_
#define __STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct stack{
	int top;			//front index of stack
	unsigned int stack_size;	//stack size
	pthread_mutex_t lock;
	void **row;					//stack table
};


struct stack* stackCreate(int stack_size);
int stackDestroy(struct stack *stack);
void* stackFat(struct stack *stack, unsigned int increase_size);

__inline__ int stackIsEmpty(struct stack *stack);
__inline__ int stackIsFull(struct stack *stack);
int stackPush(struct stack *stack, void *data);
void* stackPop(struct stack *stack);


#endif
