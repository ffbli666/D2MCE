
#ifndef __HASHTABLE_H_
#define __HASHTABLE_H_

//#include "define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//#pragma pack(push, 1)
struct hashheader{
	unsigned int id;
	char *name;
	void *next;
};
//#pragma pack(pop)

struct hashtable{
	pthread_mutex_t lock;
	unsigned short table_size;		//number of bucket
	unsigned short item_num;		//number of item;
	void **row;			//table

};

struct hashtable* hashTableCreate(int table_size);
int hashTableDestroy(struct hashtable *hashtable);
void** getHashTable(struct hashtable *hashtable);
void* hashTableSearch(struct hashtable *hashtable, int index, char *name);
//void* hashTableSearch(struct hashtable *hashtable, int hash_id);
int hashTableInsert(struct hashtable *hashtable, struct hashheader *item);


#endif

