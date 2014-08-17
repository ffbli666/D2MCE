#ifndef __SHAREMEMORY_H_
#define __SHAREMEMORY_H_

#include <semaphore.h>
#include "define.h"
#include "hashtable.h"
#include "network.h"
#include "memory.h"

//using snoop thress state protocol
enum cacheState{
	INVALID,
	SHARED,
	EXCLUSIVE
};
#ifdef CONF_SM_EVENT_DRIVER
struct sm_event_info{
	unsigned wait_count;
	sem_t sem;
};
#endif

struct sm_header{
	struct hashheader hash;
	unsigned int home_node;
	unsigned int size;
	unsigned int count;
	struct table *users;
	struct sm_info *info;
	pthread_mutex_t lock;
};

struct sm_info{
	struct sm_header *header;
	char state;
	unsigned int sync;		//number of node need guarantee to receive invalid msg
	struct sem_node_info last_node;
	struct table *need; //not invalid node , need send invalid
#ifdef CONF_SM_DISSEMINATE_UPDATE
	struct table *disseminate;
#endif
#ifdef CONF_SM_EVENT_DRIVER
	struct sm_event_info event;
#endif
#ifdef CONF_SM_AUTO_HOME_MIGARATION
	struct table *scoreboard;
#endif
};


struct sm_init_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
	unsigned int size;
};

struct sm_init_reply{
	struct request_header req;
	unsigned int home_node;
};

struct sm_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
};

struct sm_multi_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
	unsigned int offset;
	unsigned int length;
};

struct sm_ihome_reply{
	struct request_header req;
//	unsigned int count;
	unsigned int last_node;
	unsigned int count;
	unsigned int users_count;
	unsigned int need_count;
#ifdef CONF_SM_DISSEMINATE_UPDATE	
	unsigned int disseminate_count;
#endif
	//need & disseminate;
};

struct sm_newhome_req{
	struct request_header req;
	unsigned int hash_id;
	unsigned short name_len;
	unsigned int new_home;
};

void* createSM(unsigned int hash_id, char *name, unsigned int home_node, unsigned int size);
void* mallocSM(unsigned int size, char state, int ishome);
int sm_init_reply(void *request);
int sm_readmiss_reply(void *request);
int sm_fetch_reply(void *request);
int sm_writemiss_reply(void *request);
int sm_invalid_ok(void *request);
int sm_invalid_reply(void *request);
int sm_multiread_reply(void *request);
int sm_multiwrite_reply(void *request);
#ifdef CONF_SM_DISSEMINATE_UPDATE
int sm_update_register_reply(void *request);
int sm_update_unregister_reply(void *request);
int sm_update_reply(void *request);
#endif
int sm_sethome_reply(void *request);
int sm_newhome_reply(void *request);
int sm_homeready_reply(void *request);
int sm_sethome_req(struct sm_info *sm_info);


struct diff* createDiff(void* orig, void* twin, unsigned int size);
int searchNode(struct table *node, unsigned int node_id);

#ifdef CONF_SM_AUTO_HOME_MIGARATION
	int sm_uhome_reply(void *request);
	int sm_uhome_req(struct sm_info *sm_info, unsigned int send_node);
	int sm_score_add(struct sm_info *sm_info, unsigned int node_id, int addscored);
	int sm_score_zero(struct sm_info *sm_info);
#endif


#endif
