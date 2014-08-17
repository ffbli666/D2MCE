#ifndef __RECEIVER_H_
#define __RECEIVER_H_

#include <unistd.h>

#include "define.h"
#include "network.h"
#include "thread.h"
#include "ctrlThread.h"
#include "common.h"
#include "memory.h"

struct receiver_info{
	char init;
	int count;
};
struct receiver_info g_recv_info;


int receiver_init();
int receiver_destroy();
void *receiver(void *argv);
int getNodeID(unsigned int node_ip);
int receiver_close();



#endif 

