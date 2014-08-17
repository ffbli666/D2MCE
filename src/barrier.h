#ifndef __BARRIER_H_
#define __BARRIER_H_

#include <pthread.h>
#include <semaphore.h>
#include "define.h"
#include "hashtable.h"
#include "network.h"
#include "queue.h"
#include "memory.h"

struct barrier{
	struct hashheader hash;
	unsigned int wait_counter;
	char state;
	pthread_mutex_t lock;
	struct queue *queue; //unsigned short
//	struct group *group;
};

struct barrier_init_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
};

//#pragma pack(push, 2)
struct barrier_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
	unsigned int wait_counter;
};
//#pragma pack(pop)
struct ibarrier_reply{
	struct request_header req;
	unsigned int barrier_num;
};
struct barrier_info{
	unsigned int hash_id;
	unsigned short name_len;
	unsigned int wait_counter;
	char state;
	unsigned int queue_count;
	//char name
	//queue{id, seq};
};

struct barrier* createNewBarrier(unsigned int hash_id, char *name);
int insertNewBarrier(unsigned int hash_id, char *name);
int barrier_init_reply(void *request);
int barrier_reply(void *request);

int barrier_imanager_reply(void *request);
int barrier_newmanager_reply(void *request);
int barrier_imanager_req();

#endif

