#include "queue.h"

#define MAX_QUEUE_SIZE 60000

struct queue* queueCreate(int queue_size, int data_size){
	int i;
	struct queue *queue;
	queue = (struct queue*)malloc(sizeof(struct queue));
	if(queue==NULL)
		return NULL;
	queue->use = 0;
	queue->front = 0;
	queue->rear = 0;
	queue->queue_size = queue_size;
	queue->data_size = data_size;
	pthread_mutex_init(&queue->lock, NULL);
	queue->row= (void**)malloc(sizeof(void*)*queue_size);
	if(queue->row==NULL)
		return NULL;
	for(i=0; i<queue_size; i++){
		queue->row[i] = (void*)malloc(data_size);
		if(queue->row[i]==NULL)
			return NULL;
	}
	return queue;
}
int queueDestroy(struct queue *queue){
	int i;
	for(i=0; i<queue->queue_size; i++){
		if(queue->row[i]!=NULL)
			free(queue->row[i]);
	}
	if(queue->row!=NULL)
		free(queue->row);
	if(queue!=NULL)
		free(queue);
	return 1;
}
void* queueFat(struct queue *queue, unsigned int increase_size){
	int i;
	int j;
	struct queue *new_queue;
	pthread_mutex_lock(&queue->lock);
	if(queue->queue_size>=MAX_QUEUE_SIZE)
		return queue;
	new_queue = (struct queue*)malloc(sizeof(struct queue));
	if(new_queue == NULL)
		return NULL;
	new_queue->use = queue->use;
	new_queue->front = 0;
	new_queue->rear = queue->use;
	if((unsigned int)(queue->queue_size+increase_size)>MAX_QUEUE_SIZE)
		new_queue->queue_size = MAX_QUEUE_SIZE;
	else
		new_queue->queue_size = queue->queue_size + increase_size;
	new_queue->data_size = queue->data_size;
	new_queue->row= (void**)malloc(sizeof(void*)*new_queue->queue_size);
	if(new_queue->row == NULL)
		return NULL;
	for(i=0; i<new_queue->queue_size; i++){
		if(i<queue->queue_size){
			j = queue->front+i;
			if(j>=queue->queue_size)
				j -= queue->queue_size;
			new_queue->row[i] = queue->row[j];
		}
		else
			new_queue->row[i] = (void*)malloc(new_queue->data_size);
	}
	
	free(queue->row);
	pthread_mutex_unlock(&queue->lock);
	free(queue);
	return new_queue;
}

__inline__ int queueIsEmpty(struct queue *queue){
	return queue->use == 0;
}
__inline__ int queueIsFull(struct queue *queue){
	return queue->use == (queue->queue_size-1);
}
int queuePush(struct queue *queue, void *data){
//	pthread_mutex_lock(&queue->lock);
	if(!queueIsFull(queue)){
		memcpy(queue->row[queue->rear], data, queue->data_size);
		if(++queue->rear >= queue->queue_size)
			queue->rear = 0;
		queue->use++;
//		pthread_mutex_unlock(&queue->lock);
		return 1;
	}
//	pthread_mutex_unlock(&queue->lock);
	return -1;
}
int queuePushSize(struct queue *queue, void *data, int size){	
//	pthread_mutex_lock(&queue->lock);
	if(!queueIsFull(queue)){
		memcpy(queue->row[queue->rear], data, size);
		if(++queue->rear >= queue->queue_size)
			queue->rear = 0;
		queue->use++;
//		pthread_mutex_unlock(&queue->lock);		
		return 1;
	}
//	pthread_mutex_unlock(&queue->lock);
	return -1;
}

void* queueGetPush(struct queue *queue){
	void *row;
//	pthread_mutex_lock(&queue->lock);	
	if(!queueIsFull(queue)){
		row = queue->row[queue->rear];
		if(++queue->rear >= queue->queue_size)
			queue->rear = 0;
		queue->use++;
//		pthread_mutex_unlock(&queue->lock);		
		return row;
	}
//	pthread_mutex_unlock(&queue->lock);	
	return NULL;
}


void* queuePop(struct queue *queue){
	int index;
//	pthread_mutex_lock(&queue->lock);	
	if(!queueIsEmpty(queue)){
		index = queue->front;
		if(++queue->front >= queue->queue_size)
			queue->front = 0;
		queue->use--;
//		pthread_mutex_unlock(&queue->lock);
		return queue->row[index];
	}
//	pthread_mutex_unlock(&queue->lock);
	return NULL;
}



