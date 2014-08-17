#ifndef __MUTEXLOCK_H_
#define __MUTEXLOCK_H_

#include <pthread.h>
#include <semaphore.h>
#include "define.h"
#include "network.h"
#include "hashtable.h"
#include "queue.h"
#include "memory.h"

struct mutex_lock{	
	struct hashheader hash;
	char mutex;
	unsigned int owner;
	pthread_mutex_t lock;
	struct queue *queue; //unsigned short
//	struct group *group;
};

struct mutex_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
};

struct imutex_reply{
	struct request_header req;
	unsigned int mutex_num;
};

struct mutex_info{
	unsigned int hash_id;
	unsigned short name_len;
	char mutex;
	unsigned int owner;
	unsigned int queue_count;
	//char name
	//queue{id, seq};
};

struct mutex_lock* createNewMutex(unsigned int hash_id, char *name);
int insertNewMutex(unsigned int hash_id, char *name);	
int mutex_init_reply(void *request);
int mutex_lock_reply(void *request);
int mutex_unlock_reply(void *request);

int mutex_imanager_reply(void *request);
int mutex_newmanager_reply(void *request);
int mutex_imanager_req();


#endif

