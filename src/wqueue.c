#include "wqueue.h"

#define MAX_WQUEUE_SIZE 60000

struct wqueue* wqueueCreate(int wqueue_size){
	int i;
	struct wqueue *wqueue;
	wqueue = (struct wqueue*)malloc(sizeof(struct wqueue));
	if(wqueue==NULL)
		return NULL;
	wqueue->use = 0;
	wqueue->front = 0;
	wqueue->rear = 0;
	wqueue->wqueue_size = wqueue_size;
	pthread_mutex_init(&wqueue->lock, NULL);
	wqueue->row= (void**)malloc(sizeof(void*)*wqueue_size);
	if(wqueue->row==NULL)
		return NULL;
	for(i=0; i<wqueue_size; i++){
		wqueue->row[i] = NULL;
	}
	return wqueue;
}
int wqueueDestroy(struct wqueue *wqueue){
/*	int i;
	for(i=0; i<wqueue->wqueue_size; i++){
		if(wqueue->row[i]!=NULL)
			free(wqueue->row[i]);
	}
*/
	if(wqueue->row!=NULL)
		free(wqueue->row);
	if(wqueue!=NULL)
		free(wqueue);
	return 1;
}
void* wqueueFat(struct wqueue *wqueue, unsigned int increase_size){
	int i;
	int j;
	struct wqueue *new_wqueue;
	pthread_mutex_lock(&wqueue->lock);
	if(wqueue->wqueue_size>=MAX_WQUEUE_SIZE)
		return wqueue;
	new_wqueue = (struct wqueue*)malloc(sizeof(struct wqueue));
	if(new_wqueue == NULL)
		return NULL;
	new_wqueue->use = wqueue->use;
	new_wqueue->front = 0;
	new_wqueue->rear = wqueue->use;
	if((unsigned int)(wqueue->wqueue_size+increase_size)>MAX_WQUEUE_SIZE)
		new_wqueue->wqueue_size = MAX_WQUEUE_SIZE;
	else
		new_wqueue->wqueue_size = wqueue->wqueue_size + increase_size;
	new_wqueue->row= (void**)malloc(sizeof(void*)*new_wqueue->wqueue_size);
	if(new_wqueue->row == NULL)
		return NULL;
	for(i=0; i<new_wqueue->wqueue_size; i++){
		if(i<wqueue->wqueue_size){
			j = wqueue->front+i;
			if(j>=wqueue->wqueue_size)
				j -= wqueue->wqueue_size;
			new_wqueue->row[i] = wqueue->row[j];
		}
		else
			new_wqueue->row[i] = NULL;
	}
	free(wqueue->row);
	pthread_mutex_unlock(&wqueue->lock);
	free(wqueue);
	return new_wqueue;
}

__inline__ int wqueueIsEmpty(struct wqueue *wqueue){
	return wqueue->use == 0;
}
__inline__ int wqueueIsFull(struct wqueue *wqueue){
	return wqueue->use == (wqueue->wqueue_size-1);
}
int wqueuePush(struct wqueue *wqueue, void *data){
	pthread_mutex_lock(&wqueue->lock);
	if(!wqueueIsFull(wqueue)){
		wqueue->row[wqueue->rear] = data;
		if(++wqueue->rear >= wqueue->wqueue_size)
			wqueue->rear = 0;
		wqueue->use++;
		pthread_mutex_unlock(&wqueue->lock);
		return 1;
	}
	pthread_mutex_unlock(&wqueue->lock);
	return -1;
}


void* wqueuePop(struct wqueue *wqueue){
	void *data;
	pthread_mutex_lock(&wqueue->lock);	
	if(!wqueueIsEmpty(wqueue)){
		data = wqueue->row[wqueue->front];
		 wqueue->row[wqueue->front] = NULL;
		if(++wqueue->front >= wqueue->wqueue_size)
			wqueue->front = 0;
		wqueue->use--;
		pthread_mutex_unlock(&wqueue->lock);
		return data;
	}
	pthread_mutex_unlock(&wqueue->lock);
	return NULL;
}



