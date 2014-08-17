#include "stack.h"

#define MAX_STACK_SIZE 30000

struct stack* stackCreate(int stack_size){
	int i;
	struct stack *stack;
	stack = (struct stack*)malloc(sizeof(struct stack));
	if(stack==NULL)
		return NULL;
	stack->top = -1;
	stack->stack_size = stack_size;
	pthread_mutex_init(&stack->lock, NULL);
	stack->row= (void**)malloc(sizeof(void*)*stack_size);
	if(stack->row==NULL)
		return NULL;
	for(i=0; i<stack_size; i++){
		stack->row[i] = NULL;
	}	
	return stack;

}
int stackDestroy(struct stack *stack){
	/*
	int i;
	for(i=0; i<stack->stack_size; i++){
		if(stack->row[i]!=NULL)
			free(stack->row[i]);
	}
	*/
	if(stack->row!=NULL)
		free(stack->row);
	if(stack!=NULL)
		free(stack);
	return 1;
}
void* stackFat(struct stack *stack, unsigned int increase_size){
	int i;
	struct stack *new_stack;
	pthread_mutex_lock(&stack->lock);
	if(stack->stack_size>=MAX_STACK_SIZE)
		return stack;
	new_stack = (struct stack*)malloc(sizeof(struct stack));
	if(new_stack == NULL)
		return NULL;
	new_stack->top = stack->top;
	if((unsigned int)(stack->stack_size+increase_size)>MAX_STACK_SIZE)
		new_stack->stack_size = MAX_STACK_SIZE;
	else
		new_stack->stack_size = stack->stack_size + increase_size;
	new_stack->row= (void**)malloc(sizeof(void*)*new_stack->stack_size);
	if(new_stack->row == NULL)
		return NULL;
	for(i=0; i<new_stack->stack_size; i++){
		if(i<=stack->top)
			new_stack->row[i] = stack->row[i];
		else
			new_stack->row[i] = NULL;
	}
	free(stack->row);
	pthread_mutex_unlock(&stack->lock);
	free(stack);
	return new_stack;
}


__inline__ int stackIsEmpty(struct stack *stack){
	return stack->top == -1;
}
__inline__ int stackIsFull(struct stack *stack){
	return stack->top == (stack->stack_size-1);
}
//push
int stackPush(struct stack *stack, void *data){
	pthread_mutex_lock(&stack->lock);
	if(!stackIsFull(stack)){
		stack->row[++stack->top] = data;
		pthread_mutex_unlock(&stack->lock);
		return 1;
	}
	pthread_mutex_unlock(&stack->lock);
	return -1;
}
//pop
void* stackPop(struct stack *stack){
	void *data;
	pthread_mutex_lock(&stack->lock);	
	if(!stackIsEmpty(stack)){
		data = stack->row[stack->top];
		stack->top--;
		pthread_mutex_unlock(&stack->lock);
		return data;
	}
	pthread_mutex_unlock(&stack->lock);
	return NULL;
}

