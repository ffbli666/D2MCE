#include "synchronization.h"

struct synchronization g_sync;

__inline__ int sync_init(){
	g_sync.release = tableCreate(SYNC_TABLE_SIZE, sizeof(struct sync_wait));
	return 1;
}
__inline__ int sync_destroy(){
	tableDestroy(g_sync.release);
	return 1;
}

int addWait(unsigned int seq_number){
	int index;
	struct sync_wait wait;
	wait.seq_number = seq_number;
	wait.sem = NULL;
	pthread_mutex_lock(&g_sync.lock);
	index = tableGetEmpty(g_sync.release);
	if(index <0 ){
//		g_sync.release = tableFat(g_sync.release, g_sync.release->table_size);
//		index = tableGetEmpty(g_sync.release);
	}
	tableAdd(g_sync.release, (void*)&wait, index);
	pthread_mutex_unlock(&g_sync.lock);
	return 1;
}

int addWaitSem(sem_t *sem){
	int index;	
	struct sync_wait wait;
	wait.seq_number = -1;
	wait.sem = sem;
	pthread_mutex_lock(&g_sync.lock);
	index = tableGetEmpty(g_sync.release);
	if(index <0 ){
//		g_sync.release = tableFat(g_sync.release, g_sync.release->table_size);
//		index = tableGetEmpty(g_sync.release);
	}
	tableAdd(g_sync.release, (void*)&wait, index);
	pthread_mutex_unlock(&g_sync.lock);
	return 1;
}


__inline__ int acquire(){
//	pthread_mutex_lock(&g_sync.lock);
	g_sync.acquire = YES;
//	pthread_mutex_unlock(&g_sync.lock);
	return 1;
}
__inline__ int isAcquire(){
/*	int result;
	pthread_mutex_lock(&g_sync.lock);
	result = (g_sync.acquire==YES)?1:0;
	pthread_mutex_unlock(&g_sync.lock);
*/
	return (g_sync.acquire==YES)?1:0;
}
int release(){
	int i;
	void *buf;
	struct sync_wait *wait;
	pthread_mutex_lock(&g_sync.lock);
	if(g_sync.release->use>0){
		for(i=0;i<g_sync.release->use;i++){
			wait = tableGetRow(g_sync.release,i);
			if(wait->sem == NULL){
				buf = Recv(wait->seq_number);
				mem_free(buf);
			}
			else{
				sem_wait(wait->sem);
				free(wait->sem);
				wait->sem = NULL;
			}
			wait->seq_number = -1;
		}
		g_sync.release->index = 0;
		g_sync.release->use = 0;
		g_sync.acquire = NO;
	}
	pthread_mutex_unlock(&g_sync.lock);
	return 1;
}

