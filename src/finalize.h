
#ifndef __FINALIZE_H_
#define __FINALIZE_H_

#include <pthread.h>
#include <semaphore.h>

#include "define.h"
#include "network.h"
#include "table.h"

struct finalize_info{
	unsigned int count;
	struct table *node;
	pthread_mutex_t lock;
};

struct finalize_s2d_req{
	unsigned short msg_type;     		//MSG_JOIN_OK
	unsigned int group_id;				//group_id
};

struct finalize_node{
	unsigned int id;
	unsigned int seq_number;
};

int finalize_req();
int finalize_reply(struct request_header *request);

#endif
