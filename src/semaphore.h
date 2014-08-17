#ifndef __SEMAPHORE_H_
#define __SEMAPHORE_H_

#include <pthread.h>
#include <semaphore.h>
#include "define.h"
#include "network.h"
#include "hashtable.h"
#include "queue.h"
#include "memory.h"

struct sem{	
	struct hashheader hash;
	unsigned int value;
//	unsigned int owner;
	pthread_mutex_t lock;
	struct queue *queue; //unsigned short
//	struct group *group;
};
struct sem_init_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
	unsigned int value;
};

struct sem_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
};

struct isem_reply{
	struct request_header req;
	unsigned int sem_num;
};

struct sem_info{
	unsigned int hash_id;
	unsigned short name_len;
	unsigned int value;
	unsigned int queue_count;
	//char name
	//queue{id, seq};
};

struct sem* createNewSem(unsigned int hash_id, char *name, unsigned value);	
int insertNewSem(unsigned int hash_id, char *name, unsigned value);	
int sem_init_reply(void *request);
int sem_post_reply(void *request);
int sem_wait_reply(void *request);

int sem_imanager_reply(void *request);
int sem_newmanager_reply(void *request);
int sem_imanager_req();



#endif

