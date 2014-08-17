#include "memory.h"

struct memory_pool g_mem_pool;

struct stack* memory_list_init(size_t size, unsigned count);

struct stack* memory_list_init(size_t size, unsigned count){
	int i;
	struct memory_info *now;	
	struct stack *stack;
	stack = stackCreate(count);
	for(i=0;i<count;i++){
		now = malloc(size+sizeof(struct memory_info));
		now->size = size;
		stackPush(stack, now);
	}
	return stack;
}
int memory_list_destroy(struct stack *stack){
	void *buf;
	while((buf = stackPop(stack))!= NULL)
		free(buf);
	return 1;
}
int mem_init(){
	g_mem_pool.quota = MEMORY_POOL_TOTAL_SIZE;
	g_mem_pool.mem64 = memory_list_init(64, MEMORY_POOL_64_COUNT);
	g_mem_pool.mem1024 = memory_list_init(1024, MEMORY_POOL_1024_COUNT);
	g_mem_pool.mem10240 = memory_list_init(10240, MEMORY_POOL_10240_COUNT);
	g_mem_pool.memother = stackCreate(MEMORY_POOL_OTHER_COUNT);
	g_mem_pool.memtemp = stackCreate(MEMORY_POOL_OTHER_COUNT);
	return 1;
}

int mem_destroy(){
	memory_list_destroy(g_mem_pool.mem64);
	stackDestroy(g_mem_pool.mem64);
	memory_list_destroy(g_mem_pool.mem1024);
	stackDestroy(g_mem_pool.mem1024);
	memory_list_destroy(g_mem_pool.mem10240);
	stackDestroy(g_mem_pool.mem10240);
	memory_list_destroy(g_mem_pool.memother);
	stackDestroy(g_mem_pool.memother);
	return 1;
}

void* mem_malloc(size_t size){
	struct memory_info *tmp;
	struct memory_info *now;
/*
	void *buf;
	if(size <64)
		size = 64;
	buf = malloc(size+10);
	return buf;
*/
	//64
	if(size <= 64){
		now = stackPop(g_mem_pool.mem64);
//		printf("mem64=%d\n", g_mem_pool.mem64->top);
		if(now != NULL)
			return (void*)now+sizeof(struct memory_info);
	}
	//1024
	if(size <=1024){	
		now = stackPop(g_mem_pool.mem1024);
		if(now != NULL)
			return (void*)now+sizeof(struct memory_info);
	}
	//10240
	if(size <=10240){	
		now = stackPop(g_mem_pool.mem10240);
		if(now != NULL)
			return (void*)now+sizeof(struct memory_info);
	}
	//other
	while((now = stackPop(g_mem_pool.memother)) != NULL){
		if(now->size >= size){
			while((tmp = stackPop(g_mem_pool.memtemp))!=NULL)
				stackPush(g_mem_pool.memother, tmp);
			return (void*)now+sizeof(struct memory_info);
		}
		else
			stackPush(g_mem_pool.memtemp , now);
	}
	while((tmp = stackPop(g_mem_pool.memtemp))!=NULL)
		stackPush(g_mem_pool.memother, tmp);
	now = malloc(sizeof(struct memory_info)+size);
	if(now == NULL){
		printf("Error:mem_malloc malloc error\n");
		exit(1);
	}
	now->size = size;
	return (void*)now+sizeof(struct memory_info);
}

int mem_free(void* memory){
	struct memory_info *mem;
//	free(memory);
//	return 1;
	
	mem = memory - sizeof(struct memory_info);
	//64
	if(mem->size == 64){
		stackPush(g_mem_pool.mem64 , mem);
//		printf("stack use %d",g_mem_pool.mem64->top);
	}
	//1024
	else if(mem->size == 1024){
		stackPush(g_mem_pool.mem1024 , mem);
	}	
	//10240
	else if(mem->size == 10240){
		stackPush(g_mem_pool.mem10240 , mem);
	}
	//other
	else{
		stackPush(g_mem_pool.memother , mem);
	}
	return 1;
}


