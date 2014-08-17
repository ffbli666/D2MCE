#ifndef __EXIT_H_
#define __EXIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "define.h"
#include "network.h"
#include "sharememory.h"
#include "table.h"
#include "manager.h"
#include "barrier.h"
#include "mutexlock.h"
#include "semaphore.h"

struct do_manager{
	char main;			//do the main manager = YES/NO
	char mutex;
	char sem;
	char barrier;
};

struct umanager_req{
	struct request_header req;
	struct do_manager manager;
};


int chose_one(int inode_id);
int you_manager_reply(void *request);
int exit_manager_req();
int node_exit_reply(void *request);
int remove_node(unsigned int node_id);
#endif
