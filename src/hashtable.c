#include "hashtable.h"

struct hashtable* hashTableCreate(int table_size){
	int i;
	struct hashtable *hashtable;
	hashtable = (struct hashtable*)malloc(sizeof(struct hashtable));
	if(hashtable==NULL)
		return NULL;
	hashtable->table_size = table_size;
	hashtable->item_num = 0;
	pthread_mutex_init(&hashtable->lock, NULL);
	hashtable->row = (void**)malloc(sizeof(void*)*table_size);
	if(hashtable->row==NULL)
		return NULL;
	for(i=0;i<hashtable->table_size;i++){
		hashtable->row[i]=NULL;
	}
	return hashtable;
}
int hashTableDestroy(struct hashtable *hashtable){	

	int i;
	
	struct hashheader *now;
	for(i=0; i<hashtable->table_size; i++){
		while(1){
			if( hashtable->row[i]==NULL)
				break;
			now = hashtable->row[i];
			hashtable->row[i] = now->next;
			free(now);
		}
	}
	if(hashtable->row!=NULL)
		free(hashtable->row);
	if(hashtable !=NULL)
		free(hashtable);
	return 1;
}

__inline__ void** getHashTable(struct hashtable *hashtable){
	return hashtable->row;
}

void* hashTableSearch(struct hashtable *hashtable, int index, char *name){
	struct hashheader *now;
	if(hashtable->row[index]==NULL)
		return NULL;
	now = hashtable->row[index];
	while(1){
		if(strcmp(now->name, name) == 0)
			return now;
		if(now->next == NULL)
			break;
		now = now->next;
	}
	return NULL;
}

int hashTableInsert(struct hashtable *hashtable, struct hashheader *item){
	pthread_mutex_lock(&hashtable->lock);
	hashtable->item_num++;
	if(	hashtable->row[item->id]==NULL)
		hashtable->row[item->id]= item;
	else{
		item->next = hashtable->row[item->id];
		hashtable->row[item->id]= item;
	}
	pthread_mutex_unlock(&hashtable->lock);
	return 1;
}

