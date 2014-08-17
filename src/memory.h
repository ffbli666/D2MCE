/*
	memory pool
*/
#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "define.h"
#include "stack.h"

struct memory_info{
	size_t size;
};

struct memory_list{
	size_t size;
//	unsigned int count;
	unsigned int num;
	pthread_mutex_t lock;
	struct memory_info *item;
};

struct memory_pool{
	size_t quota;	//max size of memory
	//struct memory_list mem64;
	struct stack *mem64;
	struct stack *mem1024;
	struct stack *mem10240;
	struct stack *memother;
	struct stack *memtemp;
};

int mem_init();
int mem_destroy();
void* mem_malloc(size_t size);
int mem_free(void* memory);


#endif

